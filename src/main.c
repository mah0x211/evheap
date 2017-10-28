/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/main.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"


#define TOSTR(v)        TOSTRX(v)
#define TOSTRX(v)       #v

#define DEFAULT_HOST    "127.0.0.1"
#define DEFAULT_PORT    1025

typedef struct {
    const char *host;
    int port;
} cfg_t;


static void getcfg( cfg_t *cfg, int argc, char * const argv[] )
{
    const char *optstr = "p:h:";
    int opt = getopt( argc, argv, optstr );

    while( opt != -1 )
    {
        switch(opt){
            // number of workers
            // case 'w':
            //     opts.nproc = atoi(optarg);
            //     break;

            // host
            case 'h':
                cfg->host = optarg;
                break;

            // port number
            case 'p':
                cfg->port = atoi( optarg );
                if( cfg->port >= 0 && cfg->port <= 0xFFFF ){
                    break;
                }
                printf( "invalid port-range\n" );

            default:
                printf(
                    "Usage: evheap [-p port] [-h host]\n"
                    "default opts\n"
                    "   host | " TOSTR(DEFAULT_HOST) "\n"
                    "   port | " TOSTR(DEFAULT_PORT) "\n"
                    "\n"
                );
                exit( EXIT_FAILURE );
        }

        opt = getopt( argc, argv, optstr );
    }
}


int main( int argc, char * const argv[] )
{
    server_t s = server_init();
    int ch = 0;
    cfg_t cfg = {
        .host = DEFAULT_HOST,
        .port = DEFAULT_PORT,
    };

    getcfg( &cfg, argc, argv );

    // disable buffering
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );

    if( hctx_alloc( &s.ctx ) == -1 ){
        perror("failed to hctx_alloc()");
        return EXIT_FAILURE;
    }
    else if( ( ch = sigch_init() ) == -1 ){
        perror("failed to sigch_init()");
        return EXIT_FAILURE;
    }

    // create server socket
    if( server_listen( &s, cfg.host, cfg.port ) == 0 )
    {
        char strbuf[SERVER_ADDRLEN] = {0};
        int sig = 0;

        printf( "server listening on %s:%d\n", server_addr( &s, strbuf ),
                server_port( &s ) );

        // wait signal
        while( chrecv( ch, (void*)&sig, sizeof( sig ), -1 ) == 0 ){
            printf("got signal %d\n", sig );
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

