/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/dynarr.h
  Created by Masatoshi Teruya on 17/10/27

*/

#ifndef dynarr_h
#define dynarr_h

#include <stdlib.h>


typedef struct {
    size_t len;
    int *arr;
} dynarr_t;

#define dynarr_initializer {    \
    .len = 0,                   \
    .arr = NULL                 \
}

int dynarr_alloc( dynarr_t *map, size_t len );
void dynarr_dealloc( dynarr_t *map );
int dynarr_get( dynarr_t *map, int idx );
int dynarr_del( dynarr_t *map, int idx );
int dynarr_set( dynarr_t *map, int idx, int val );


#endif /* dynarr_h */
