/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/evheap.h
  Created by Masatoshi Teruya on 17/10/25

*/

#ifndef evheap_h
#define evheap_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hctx.h"


int sigch_init( void );


/**
 *  server data and related functions
 */
typedef struct {
    int h;
    int sock;
    struct ipaddr addr;
    hctx_t ctx;
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
 *  e.g.
 *    char buf[SERVER_ADDRLEN] = {0};
 *    printf( "host: %s:%d\n", server_addr( &s, buf ), server_port( &s ) );
 */
#define SERVER_ADDRLEN      IPADDR_MAXSTRLEN
#define server_addr(s, buf) ipaddr_str(&(s)->addr, (buf))
#define server_port(s)      ipaddr_port( &(s)->addr )


/**
 *  client data and related functions
 */
coroutine void client_co( hctx_t *ctx, int id, int sock );


/**
 * Message Format Specification
 *
 *  ---------+----------+---------+----------
 *  message header: 8 octets
 *  ---------+----------+---------+----------
 *  message body: 4 or 8 octets + payload
 *  ---------+----------+---------+----------
 *
 *
 *  message header
 *  ---------+----------+---------+----------
 *  type     | id ...
 *  ---------+----------+---------+----------
 *  1 octet    7 octets
 *
 *  type values
 *  --------+------------ for Response
 *  ok      | 0000 0001 | 0x1
 *  data    | 0000 0010 | 0x2
 *  err     | 0000 0011 | 0x3
 *  --------+------------ for Request
 *  close   | 1000 0000 | 0x80
 *  ping    | 1000 0001 | 0x81
 *  push    | 1000 0010 | 0x82
 *  pull    | 1000 0011 | 0x83
 *  --------+------------
 *
 *
 *  message body 4 octets + payload
 *  ---------+----------+----------+---------
 *  key-size            | payload ...
 *  ---------+----------+----------+---------
 *
 *  message body 4 + 4 octets + payload
 *  ---------+----------+----------+---------
 *  key-size            | val-size
 *  ---------+----------+----------+---------
 *  payload ...
 *  ---------+----------+----------+---------
 *
 *
 *  list of byte sequences
 *  --------+----+-----+-----+----+----
 *  close   | id
 *  --------+----+-----+-----+----+----
 *  ping    | id
 *  --------+----+-----+-----+----+----
 *  pull    | id | ksz | payload
 *  --------+----+-----+-----+----+----
 *  push    | id | ksz | vsz | payload
 *  --------+----+-----+-----+----+----
 *  ok      | id
 *  --------+----+-----+-----+----+----
 *  err     | id | vsz | payload
 *  --------+----+-----+-----+----+----
 *  data    | id | vsz | payload
 *  --------+----+-----+-----+----+----
 */
enum {
    // response
    MSG_RES_OK = 0x1,
    MSG_RES_DATA,
    MSG_RES_ERR,
    // request
    MSG_REQ_CLOSE = 0x80,
    MSG_REQ_PING,
    MSG_REQ_PUSH,
    MSG_REQ_PULL
};


/**
 *  message byte sequences
 *  --------+----+-----+-----+----+----
 *  close   | id
 *  --------+----+-----+-----+----+----
 *  ping    | id
 *  --------+----+-----+-----+----+----
 *  ok      | id
 *  --------+----+-----+-----+----+----
 */
typedef struct __attribute__((packed)){
    uint64_t tid;
} msg_oct8_t;
typedef msg_oct8_t msg_hdr_t;
typedef msg_oct8_t msg_ok_t;
typedef msg_oct8_t msg_ping_t;


/**
 *  message byte sequences
 *  --------+----+-----+-----+----+----
 *  err     | id | vsz | payload
 *  --------+----+-----+-----+----+----
 *  data    | id | vsz | payload
 *  --------+----+-----+-----+----+----
 *  pull    | id | ksz | payload
 *  --------+----+-----+-----+----+----
 */
typedef struct __attribute__((packed)){
    uint64_t tid;
    uint32_t klen;
} msg_oct12_t;

typedef msg_oct12_t msg_err_t;
typedef msg_oct12_t msg_data_t;
typedef msg_oct12_t msg_pull_t;


/**
 *  message byte sequences
 *  --------+----+-----+-----+----+----
 *  push    | id | ksz | vsz | payload
 *  --------+----+-----+-----+----+----
 */
typedef struct __attribute__((packed)){
    uint64_t tid;
    uint32_t klen;
    uint32_t vlen;
} msg_oct16_t;

typedef msg_oct16_t msg_push_t;
typedef msg_oct16_t msg_t;


/**
 *  message input/output functions
 */
void *msgio_recv( int sock, int64_t deadline, uint8_t *type );
int msgio_send( int sock, void *data, int64_t deadline, size_t *sent );


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


#endif
