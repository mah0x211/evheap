/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/rawio.h
  Created by Masatoshi Teruya on 17/10/30

*/

#ifndef rawio_h
#define rawio_h

#include "evheap.h"
#include <sys/types.h>
#include <sys/socket.h>


/**
 *  bytestream input/output funcitons
 *
 *  e.g.
 *    char buf[BUFSIZ] = { 0 };
 *    int sock = tcp_accept( s, NULL, -1 );
 *    ssize_t len = rawio_recv( sock, buf, BUFSIZ, deadline );
 *    ssize_t sent = rawio_send( sock, buf, len, deadline );
 */
ssize_t rawio_recv( int sock, char *buf, size_t len, int64_t deadline );
ssize_t rawio_send( int sock, char *buf, size_t len, int64_t deadline );
#define rawio_recvn(sock, buf, len, deadline) brecv( sock, buf, len, deadline)


#endif /* rawio_h */

