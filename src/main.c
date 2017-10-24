/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/main.c
  Created by Masatoshi Teruya on 17/10/25

*/

#include "evheap.h"

coroutine static void worker(const char *text)
{
    while(1) {
        printf("%s\n", text);
        msleep(now() + random() % 500);
    }
}


int main( int argc, const char *argv[] )
{
    (void)argc;
    (void)argv;

    // disable buffering
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );

    go( worker("hello world!") );
    msleep( now() + 1000 );
    printf("done\n");

    return EXIT_SUCCESS;
}

