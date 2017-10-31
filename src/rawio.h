/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/rawio.h
  Created by Masatoshi Teruya on 17/10/30

*/

#ifndef rawio_h
#define rawio_h

#include <sys/types.h>


typedef struct {
    size_t cap;
    size_t tail;
    size_t head;
    char *buf;
    int fd;
} rawio_t;

#define rawio_initializer { \
    .cap = 0,               \
    .tail = 0,              \
    .head = 0,              \
    .buf = NULL,            \
    .fd = -1                \
}

int rawio_init( rawio_t *r, int sock, size_t cap );
void rawio_dispose( rawio_t *r );


/**
 *  bytestream input/output funcitons
 *
 *  e.g.
 *    char buf[BUFSIZ] = { 0 };
 *    int sock = tcp_accept( s, NULL, -1 );
 *    ssize_t len = rawio_recv( sock, buf, BUFSIZ, deadline );
 *    ssize_t sent = rawio_send( sock, buf, len, deadline );
 */
int rawio_recv( rawio_t *r, char *buf, size_t *len, int64_t deadline );
int rawio_recvn( rawio_t *r, char *buf, size_t *len, int64_t deadline );
ssize_t rawio_send( rawio_t *r, char *buf, size_t len, int64_t deadline );


#endif /* rawio_h */

