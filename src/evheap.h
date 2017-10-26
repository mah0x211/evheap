/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/evheap.h
  Created by Masatoshi Teruya on 17/10/25

*/

#ifndef evheap_h
#define evheap_h

#include "../deps/libdill/libdill.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int sigch_init( void );


typedef struct {
    int h;
    int sock;
    struct ipaddr addr;
} server_t;


#define server_init()   \
((server_t){            \
    .h = -1,            \
    .sock = -1,         \
    .addr = {           \
        .data = {0}     \
    }                   \
})

int server_listen( server_t *s, const char *host, int port );
void server_close( server_t *s );


/**
 * e.g.
 *
 *  char strbuf[SERVER_ADDRLEN] = {0};
 *  printf( "hostname: %s\n", server_addr( &s, strbuf );
 */
#define SERVER_ADDRLEN      IPADDR_MAXSTRLEN
#define server_addr(s, buf) ipaddr_str(&(s)->addr, (buf))
#define server_port(s)      ipaddr_port( &(s)->addr )


ssize_t rawio_recv( int sock, char *buf, size_t len, int64_t deadline );
ssize_t rawio_send( int sock, char *buf, size_t len, int64_t deadline );
#define rawio_recvn(sock, buf, len, deadline) brecv( sock, buf, len, deadline)


#endif
