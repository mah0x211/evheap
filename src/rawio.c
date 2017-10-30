/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/rawio.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "rawio.h"


DILL_EXPORT extern int tcp_fd(int s);


#define is_eagain() (errno == EAGAIN || errno == EWOULDBLOCK)


ssize_t rawio_recv( int sock, char *buf, size_t len, int64_t deadline )
{
    int fd = tcp_fd( sock );
    ssize_t rv = 0;

RECV_AGAIN:
    rv = recv( fd, buf, len, 0 );
    switch( rv )
    {
        case -1:
            if( errno == EINTR || ( is_eagain() && fdin( fd, deadline ) == 0 ) ){
                goto RECV_AGAIN;
            }

        default:
            return rv;
    }
}


ssize_t rawio_send( int sock, char *buf, size_t len, int64_t deadline )
{
    int fd = tcp_fd( sock );
    ssize_t rv = 0;

SEND_AGAIN:
    rv = send( fd, buf, len, 0 );
    switch( rv )
    {
        case -1:
            if( errno == EINTR ||
                ( is_eagain() && fdout( fd, deadline ) == 0 ) ){
                goto SEND_AGAIN;
            }
        case 0:
            return rv;

        default:
            len -= (size_t)rv;
            if( len ){
                goto SEND_AGAIN;
            }
            return rv;
    }
}

