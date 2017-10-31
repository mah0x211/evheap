/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/client.c
  Created by Masatoshi Teruya on 17/10/28

*/

#include "evheap.h"


coroutine static void echo_co( hctx_t *ctx, int id, rawio_t *r, void *data,
                               int64_t deadline )
{
    size_t sent = 0;
    int rv = msgio_send( r, data, deadline, &sent );

    free( data );
    if( rv == -1 )
    {
        // if not closed by peer
        switch( errno )
        {
            // no error
            case 0:
                break;

            // Deadline was reached.
            case ETIMEDOUT:
                // if( data ){
                //     msend( sock, "timed-out", -1, -1 );
                // }
                break;

            // Not enough memory.
            case ENOMEM:
                // if( data ){
                //     msend( sock, "internal server error", -1, -1 );
                // }
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
    }

    handle_close( ctx, id );
}


coroutine void client_co( hctx_t *ctx, int id, int sock )
{
    hctx_t cctx = hctx_initializer;

    if( hctx_init( &cctx ) == 0 )
    {
        rawio_t r = rawio_initializer;

        if( rawio_init( &r, sock, 4096 ) == 0 )
        {
            int64_t deadline = -1;
            uint8_t type = 0;
            void *data = NULL;

RECV_NEXT:
            if( ( data = msgio_recv( &r, deadline, &type ) ) )
            {
                switch( type ){
                    case MSG_REQ_PING:
                    case MSG_REQ_PULL:
                    case MSG_REQ_PUSH:
                        if( handle( &cctx, echo_co, &r, data, deadline ) == 0 ){
                            goto RECV_NEXT;
                        }
                        break;

                    case MSG_REQ_CLOSE:
                        break;

                    default:
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

            if( data ){
                free( data );
            }
            // TODO: wait until all child coroutine finished

            rawio_dispose( &r );
        }
        else {
            perror("failed to rawio_init()");
        }

        hctx_dispose( &cctx );
    }
    else {
        perror("failed to hctx_init()");
    }

    if( hclose( sock ) == -1 ){
        perror("failed to hclose()");
    }
    handle_close( ctx, id );
}


