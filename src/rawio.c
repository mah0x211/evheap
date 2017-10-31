/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/rawio.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "rawio.h"
#include <string.h>
#include <sys/socket.h>
#include "libdill.h"


DILL_EXPORT extern int tcp_fd(int s);


int rawio_init( rawio_t *r, int sock, size_t cap )
{
    if( ( r->buf = malloc( cap ) ) ){
        r->cap = cap;
        r->tail = 0;
        r->head = 0;
        r->fd = tcp_fd( sock );
        return 0;
    }

    return -1;
}


void rawio_dispose( rawio_t *r )
{
    free( r->buf );
}


#define is_eagain() (errno == EAGAIN || errno == EWOULDBLOCK)


static inline ssize_t fetch( int fd, char *buf, size_t len, int64_t deadline )
{
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


ssize_t rawio_send( rawio_t *r, char *buf, size_t len, int64_t deadline )
{
    ssize_t rv = 0;

SEND_AGAIN:
    rv = send( r->fd, buf, len, 0 );
    switch( rv )
    {
        case -1:
            if( errno == EINTR ||
                ( is_eagain() && fdout( r->fd, deadline ) == 0 ) ){
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

#undef is_eagain


int rawio_recv( rawio_t *r, char *buf, size_t *blen, int64_t deadline )
{
    size_t len = *blen;
    size_t remain = r->tail - r->head;

    if( len > remain )
    {
        ssize_t rv = 0;

        // copy existing data
        if( remain ){
            memcpy( buf, r->buf + r->head, remain );
            buf += r->head;
            r->tail = 0;
            r->head = 0;
            len -= remain;
        }

        // if buffer-length larger than capacity size, use passed buffer directly
        if( len >= r->cap )
        {
            rv = fetch( r->fd, buf, len, deadline );
            if( rv > 0 ){
                *blen -= len - rv;
                return 1;
            }

            return rv;
        }

        rv = fetch( r->fd, r->buf, r->cap, deadline );
        if( rv > 0 )
        {
            if( (size_t)rv > len ){
                memcpy( buf, r->buf, len );
                r->tail = rv;
                r->head = len;
            }
            else {
                memcpy( buf, r->buf, rv );
                *blen -= len - rv;
            }

            return 1;
        }

        return rv;
    }

    // copy existing data
    memcpy( buf, r->buf + r->head, len );
    r->head += len;

    return 1;
}


int rawio_recvn( rawio_t *r, char *buf, size_t *blen, int64_t deadline )
{
    size_t remain = *blen;
    size_t len = remain;
    ssize_t rv = 0;

RECV_AGAIN:
    rv = rawio_recv( r, buf, &len, deadline );
    if( rv > 0 && ( remain -= len ) ){
        buf += len;
        len = remain;
        goto RECV_AGAIN;
    }

    *blen -= remain;

    return rv;
}

