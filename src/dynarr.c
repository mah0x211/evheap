/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/dynarr.c
  Created by Masatoshi Teruya on 17/10/27

*/

#include "dynarr.h"
#include <string.h>
#include <limits.h>
#include <errno.h>


#define VALSIZE sizeof(int)


int dynarr_init( dynarr_t *da, size_t len )
{
    da->arr = calloc( len, VALSIZE );
    if( da->arr ){
        da->len = len;
        return 0;
    }

    return -1;
}


void dynarr_dealloc( dynarr_t *da )
{
    free( (void*)da->arr );
}


int dynarr_get( dynarr_t *da, int idx )
{
    if( idx < 0 || (size_t)idx > da->len ){
        return -1;
    }

    return da->arr[idx] - 1;
}


int dynarr_del( dynarr_t *da, int idx )
{
    if( idx < 0 || (size_t)idx > da->len ){
        return -1;
    }

    da->arr[idx] = 0;

    return 0;
}


static int resize( dynarr_t *da, int idx )
{
    if( idx < 0 ){
        return -1;
    }
    else if( (size_t)idx >= da->len )
    {
        // realloc idx container
        int *arr  = realloc( da->arr, ( idx + 1 ) * VALSIZE );

        if( !arr ){
            return -1;
        }
        memset( arr + idx, 0, VALSIZE );
        da->len = idx;
        da->arr = arr;
    }

    return 0;
}


int dynarr_set( dynarr_t *da, int idx, int val )
{
    if( val < 0 || val >= INT_MAX ){
        errno = EINVAL;
    }
    // autoresize
    else if( resize( da, idx ) == 0 ){
        da->arr[idx] = val + 1;
        return 0;
    }

    return -1;
}

