/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/server.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"
#include "../deps/libdill/libdillimpl.h"


coroutine static void handle_connection( int sock )
{
    int64_t deadline = -1;
    ssize_t len = 0;
    char buf[BUFSIZ] = { 0 };

    while(1)
    {
        len = mrecv( sock, buf, sizeof( buf ), deadline );
        if( len != -1 && msend( sock, buf, len, -1 ) == 0 ){
            continue;
        }
        break;
    }

    switch( errno )
    {
        // Deadline was reached.
        case ETIMEDOUT:
            if( len == -1 ){
                msend( sock, "timed-out", -1, -1 );
            }
            break;

        // Not enough memory.
        case ENOMEM:
            if( len == -1 ){
                msend( sock, "internal server error", -1, -1 );
            }
            break;

        // Current coroutine is in the process of shutting down.
        case ECANCELED:
            perror("coroutine canceled");
            break;

        // Closed connection.
        case EPIPE:
        // Broken connection.
        case ECONNRESET:
            // TODO: print only on debug mode
            // perror("connection closed by peer");
            break;

        // The socket handle in invalid.
        // case EBADF:
        // Invalid arguments.
        // case EINVAL:
        // The message was larger than the supplied buffer.
        // case EMSGSIZE:
        // The operation is not supported by the socket.
        // case ENOTSUP:
        default:
            perror("failed to handle_connection()");
    }

    if( hclose( sock ) == -1 ){
        perror("failed to hclose()");
    }
}


coroutine static void handle_accept( server_t *s )
{
    while(1)
    {
        // accept socket
        int sock = tcp_accept( s->sock, NULL, -1 );

        if( sock != -1 )
        {
            int cr = crlf_attach( sock );

            if( cr != -1 )
            {
                sock = cr;
                if( go( handle_connection( sock ) ) != -1 ){
                    continue;
                }
                crlf_detach( cr, -1 );
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
        s->h = go( handle_accept( s ) );
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
}

