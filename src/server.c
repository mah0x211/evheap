/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/server.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"


coroutine static void handle_connection( hctx_t *ctx, int id, int sock )
{
    int64_t deadline = -1;
    size_t sent = 0;
    uint8_t type = 0;
    msg_hdr_t *data = NULL;
    void *next = &&RECV_AGAIN;
    int rv = 0;

RECV_AGAIN:
    if( ( data = msgio_recv( sock, deadline, &type ) ) )
    {
        switch( type ){
            case MSG_REQ_CLOSE:
                next = &&HANDLE_CLOSE;
            case MSG_REQ_PING:
            case MSG_REQ_PULL:
            case MSG_REQ_PUSH:
                rv = msgio_send( sock, data, deadline, &sent );
                if( rv == 0 ){
                    free( data );
                    goto *next;
                }

            default:
                free( data );
                errno = EBADMSG;
        }
    }

    // if not closed by peer
    switch( errno )
    {
        // no error
        case 0:
            break;

        // Deadline was reached.
        case ETIMEDOUT:
            if( data ){
                msend( sock, "timed-out", -1, -1 );
            }
            break;

        // Not enough memory.
        case ENOMEM:
            if( data ){
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

HANDLE_CLOSE:
    if( hclose( sock ) == -1 ){
        perror("failed to hclose()");
    }
    handle_close( ctx, id );
}


coroutine static void handle_accept( server_t *s )
{
    while(1)
    {
        // accept socket
        int sock = tcp_accept( s->sock, NULL, -1 );

        if( sock != -1 )
        {
            if( handle( &s->ctx, handle_connection, sock ) == 0 ){
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
    hctx_dealloc( &s->ctx );
}

