/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/main.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"


int main( int argc, const char *argv[] )
{
    int ch = sigch_init();
    int signo = 0;

    if( ch == -1 ){
        perror("failed to initialize sigch");
        exit(1);
    }

    (void)argc;
    (void)argv;

    // disable buffering
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );

    // wait signal
    while( chrecv( ch, (void*)&signo, sizeof(signo), -1 ) == 0 ){
        printf("got signal %d\n", signo );
        break;
    }

    hclose( ch );
    printf("chdone\n");

    // hclose( server );
    printf("done\n");

    return EXIT_SUCCESS;
}


