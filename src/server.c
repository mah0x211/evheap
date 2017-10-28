/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/server.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"


coroutine static void accept_co( server_t *s )
{
    while(1)
    {
        // accept socket
        int sock = tcp_accept( s->sock, NULL, -1 );

        if( sock != -1 )
        {
            if( handle( &s->ctx, client_co, sock ) == 0 ){
                continue;
            }
            hclose( sock );
        }

        switch( errno )
        {
            // The maximum number of file descriptors in the process are already open.
            case EMFILE:
            // The maximum number of file descriptors in the system are already open.
            case ENFILE:
            // Not enough memory.
            case ENOMEM:
                perror("warn");
                continue;

            // Invalid socket handle.
            // case EBADF:
            // Current coroutine is being shut down.
            // case ECANCELED:
            // ETIMEDOUT: Deadline was reached.
            default:
                perror("can't accept socket");
        }
        break;
    }
}


int server_listen( server_t *s, const char *host, int port )
{
    if( ipaddr_remote( &s->addr, host, port, 0, -1 ) == 0 &&
        ( s->sock = tcp_listen( &s->addr, -1 ) ) != -1 )
    {
        s->h = go( accept_co( s ) );
        if( s->h != -1 ){
            return 0;
        }
        hclose( s->sock );
    }

    return -1;
}


void server_close( server_t *s )
{
    hclose( s->h );
    hclose( s->sock );
    hctx_dealloc( &s->ctx );
}

