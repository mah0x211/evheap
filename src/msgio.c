/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/msgio.c
  Created by Masatoshi Teruya on 17/10/26

*/

#include "evheap.h"


static inline void *recv_oct16( uint64_t tid, int sock, int64_t deadline )
{
    char hdr[8] = { 0 };

    if( rawio_recvn( sock, hdr, 8, deadline ) == 0 )
    {
        uint32_t klen = ntohl( *(uint32_t*)hdr );
        uint32_t vlen = ntohl( *(uint32_t*)( hdr + 4 ) );
        size_t len = klen + vlen;
        msg_oct16_t *msg = malloc( sizeof( msg_oct16_t ) + len );

        if( msg )
        {
            char *payload = (char*)msg + sizeof( msg_oct16_t );

            msg->tid = tid;
            msg->klen = klen;
            msg->vlen = vlen;
            if( rawio_recvn( sock, payload, len, deadline ) == 0 ){
                return (void*)msg;
            }
            free( (void*)msg );
        }
    }

    return NULL;
}


static inline void *recv_oct12( uint64_t tid, int sock, int64_t deadline )
{
    char hdr[4] = { 0 };

    if( rawio_recvn( sock, hdr, 4, deadline ) == 0 )
    {
        uint32_t klen = ntohl( *(uint32_t*)hdr );
        msg_oct12_t *msg = malloc( sizeof( msg_oct12_t ) + klen );

        if( msg )
        {
            void *payload = (char*)msg + sizeof( msg_oct12_t );

            msg->tid = tid;
            msg->klen = klen;
            if( rawio_recvn( sock, payload, klen, deadline ) == 0 ){
                return (void*)msg;
            }
            free( (void*)msg );
        }
    }

    return NULL;
}


static inline void *recv_oct8( uint64_t tid )
{
    msg_oct8_t *msg = malloc( sizeof( msg_oct8_t ) );

    if( msg ){
        msg->tid = tid;
        return (void*)msg;
    }

    return NULL;
}


void *msgio_recv( int sock, int64_t deadline, uint8_t *type )
{
    char buf[8] = { 0 };

    errno = 0;
    if( rawio_recvn( sock, buf, 8, deadline ) == 0 )
    {
        uint64_t tid = *(uint64_t*)buf;

        *type = buf[0];
        switch( (uint8_t)*buf ){
            case MSG_RES_OK:
            case MSG_REQ_CLOSE:
            case MSG_REQ_PING:
                return recv_oct8( tid );

            case MSG_RES_DATA:
            case MSG_RES_ERR:
            case MSG_REQ_PULL:
                return recv_oct12( tid, sock, deadline );

            case MSG_REQ_PUSH:
                return recv_oct16( tid, sock, deadline );

            // invalid message type
            default:
                errno = EBADMSG;
        }
    }

    return NULL;
}


static inline int send_data( int sock, void *data, size_t len, int64_t deadline,
                             size_t *sent )
{
    ssize_t rv = 0;
    size_t total = 0;

SEND_AGAIN:
    rv = rawio_send( sock, data, len, deadline );
    if( rv > 0 )
    {
        total += (size_t)rv;
        len -= rv;
        if( len ){
            goto SEND_AGAIN;
        }

        *sent = total;
        return 0;
    }

    *sent = total;
    return -1;
}


int msgio_send( int sock, void *data, int64_t deadline, size_t *sent )
{
    uint8_t type = *(char*)data;
    msg_oct12_t *msg12 = NULL;
    msg_oct16_t *msg16 = NULL;
    size_t len = 0;
    int rv = 0;

    // check response type
    switch( type ){
        case MSG_RES_OK:
        case MSG_REQ_CLOSE:
        case MSG_REQ_PING:
            return send_data( sock, data, sizeof( msg_oct8_t ), deadline, sent );

        case MSG_RES_ERR:
        case MSG_RES_DATA:
        case MSG_REQ_PULL:
            msg12 = (msg_oct12_t*)data;
            len = msg12->klen + sizeof( msg_oct12_t );
            msg12->klen = htonl( msg12->klen );
            rv = send_data( sock, data, len, deadline, sent );
            msg12->klen = ntohl( msg12->klen );
            return rv;

        case MSG_REQ_PUSH:
            msg16 = (msg_oct16_t*)data;
            len = msg16->klen + msg16->vlen + sizeof( msg_oct16_t );
            msg16->klen = htonl( msg16->klen );
            msg16->vlen = htonl( msg16->vlen );
            rv = send_data( sock, data, len, deadline, sent );
            msg16->klen = ntohl( msg16->klen );
            msg16->vlen = ntohl( msg16->vlen );
            return rv;

        // invalid message type
        default:
            errno = EBADMSG;
            return -1;
    }
}


