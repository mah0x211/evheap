/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/sigch.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"
#include <signal.h>
#include <stdarg.h>


static int FD_RCV = -1;
static int FD_SND = -1;


coroutine static void recvsig( int ch )
{
    while(1)
    {
        int sig = -1;

        // wait signal
        if( fdin( FD_RCV, -1 ) == 0 ){
            read( FD_RCV, &sig, sizeof( sig ) );
        }

        if( chsend( ch, (void*)&sig, sizeof( sig ), -1 ) != 0 || sig == -1 ){
            perror("failed to recvsig()");
            close( FD_RCV );
            FD_RCV = -1;
            break;
        }
    }
}


static void sendch( int sig )
{
    // forward signo to pipe
    if( write( FD_SND, (void*)&sig, sizeof( sig ) ) == -1 ){
        perror("failed to write(FD_SND)");
        close( FD_SND );
        FD_SND = -1;
        close( FD_RCV );
    }
}


static int setsigact( int sig )
{
    struct sigaction act;

    memset( (void*)&act, 0, sizeof( act ) );
    act.sa_handler = sendch;
    act.sa_flags = SA_RESTART;

    return sigaction( sig, &act, NULL );
}


int sigch_init( int sig, ... )
{
    int fds[2] = { -1, -1 };

    if( pipe( fds ) != -1 )
    {
        int ch = 0;
        sigset_t sblk;
        va_list ap;

        FD_RCV = fds[0];
        FD_SND = fds[1];

        sigfillset( &sblk );
        va_start( ap, sig );
        do
        {
            if( sigdelset( &sblk, sig ) == -1 ){
                perror( "failed to sigdelset()" );
                va_end( ap );
                goto FAIL;
            }
            else if( setsigact( sig ) == -1 ){
                perror( "failed to setsigact()" );
                va_end( ap );
                goto FAIL;
            }
        } while( ( sig = va_arg( ap, int ) ) );
        va_end( ap );

        if( sigprocmask( SIG_BLOCK, &sblk, NULL ) == 0 &&
            ( ch = chmake( sizeof( ch ) ) ) != -1 )
        {
            // wait a signal with coroutine
            if( go( recvsig( ch ) ) != -1 ){
                return ch;
            }
            hclose( ch );
        }

FAIL:
        close( FD_RCV );
        close( FD_SND );
    }

    return -1;
}


