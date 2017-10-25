/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/sigch.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"
#include <signal.h>


static int FD_RCV = -1;
static int FD_SND = -1;


coroutine static void recvsig( int ch )
{
    while(1)
    {
        int signo = -1;

        // wait signal
        if( fdin( FD_RCV, -1 ) == 0 ){
            read( FD_RCV, &signo, sizeof(signo) );
        }

        if( chsend( ch, (void*)&signo, sizeof(int), -1 ) != 0 || signo == -1 ){
            perror("failed to recvsig()");
            close( FD_RCV );
            FD_RCV = -1;
            break;
        }
    }
}


static void sendch( int signo )
{
    // forward signo to pipe
    if( write( FD_SND, (void*)&signo, sizeof(int) ) == -1 ){
        perror("failed to write(FD_SND)");
        close( FD_SND );
        FD_SND = -1;
        close( FD_RCV );
    }
}


int sigch_init( void )
{
    int fds[2] = { -1, -1 };

    if( pipe( fds ) != -1 )
    {
        int ch = 0;
        struct sigaction act;
        sigset_t sblk;

        FD_RCV = fds[0];
        FD_SND = fds[1];

        memset( (void*)&act, 0, sizeof( struct sigaction ) );
        act.sa_handler = sendch;
        act.sa_flags = SA_RESTART;

        sigfillset( &sblk );
            // block all signal except SIGINT
        if( sigdelset( &sblk, SIGINT ) == 0 &&
            sigprocmask( SIG_BLOCK, &sblk, NULL ) == 0 &&
            sigaction( SIGINT, &act, NULL ) != -1 &&
            // create channel
            ( ch = chmake(sizeof(int)) ) != -1 )
        {
            // wait a signal with coroutine
            if( go( recvsig( ch ) ) != -1 ){
                return ch;
            }
            hclose( ch );
        }

        close( FD_RCV );
        close( FD_SND );
    }

    return -1;
}


