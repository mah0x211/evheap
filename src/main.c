/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/main.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"


int main( int argc, const char *argv[] )
{
    server_t s = server_init();
    int ch = 0;

    (void)argc;
    (void)argv;
    // disable buffering
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );

    if( ( ch = sigch_init() ) == -1 ){
        perror("failed to sigch_init()");
        return EXIT_FAILURE;
    }

    // create server socket
    if( server_listen( &s, "127.0.0.1", 1025 ) == 0 )
    {
        char strbuf[SERVER_ADDRLEN] = {0};
        int signo = 0;

        printf( "server listening on %s:%d\n", server_addr( &s, strbuf ),
                server_port( &s ) );

        // wait signal
        while( chrecv( ch, (void*)&signo, sizeof(signo), -1 ) == 0 ){
            printf("got signal %d\n", signo );
            break;
        }

        server_close( &s );
        printf("done\n");

        return EXIT_SUCCESS;
    }

    hclose( ch );
    perror("failed to server_run()");

    return EXIT_FAILURE;
}


