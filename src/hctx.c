/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/hctx.c
  Created by Masatoshi Teruya on 17/10/27

*/

#include "hctx.h"
#include "libdill.h"
#include <limits.h>
#include <errno.h>


#define DEFUALT_SIZE    4096


int hctx_alloc( hctx_t *ctx )
{
    if( bitvec_init( &ctx->ids, DEFUALT_SIZE ) == 0 )
    {
        if( dynarr_init( &ctx->tbl, DEFUALT_SIZE ) == 0 ){
            ctx->used = -1;
            return 0;
        }
        bitvec_dispose( &ctx->ids );
    }

    return -1;
}


void hctx_dealloc( hctx_t *ctx )
{
    bitvec_dispose( &ctx->ids );
    dynarr_dealloc( &ctx->tbl );
}


int hctx_get( hctx_t *ctx )
{
    size_t id = 0;

    // resize
    if( bitvec_get( &ctx->ids, ctx->ids.nbit ) == 1 &&
        bitvec_resize( &ctx->ids, ctx->ids.nbit + 1 ) == -1 ){
        return -1;
    }
    // limit exceeded
    else if( ( id = bitvec_ffz( &ctx->ids ) ) > (size_t)INT_MAX ){
        errno = EAGAIN;
        return -1;
    }
    // probably, bug in internal implementation
    else if( dynarr_get( &ctx->tbl, id ) != -1 ||
             bitvec_set( &ctx->ids, id ) != 0 ){
        errno = ENOTRECOVERABLE;
        return -1;
    }

    return (int)id;
}


int hctx_set( hctx_t *ctx, int id, int h )
{
    // invalid argument
    if( h < 0 ){
        errno = EINVAL;
    }
    // invalid id
    else if( bitvec_get( &ctx->ids, id ) != 1 ){
        errno = ENOENT;
    }
    // already exists
    else if( dynarr_get( &ctx->tbl, id ) >= 0 ){
        errno = EEXIST;
    }
    else if( dynarr_set( &ctx->tbl, id, h ) == 0 ){
        return 0;
    }

    return -1;
}


void hctx_del( hctx_t *ctx, int id )
{
    // release used handle
    if( ctx->used != -1 ){
        int h = dynarr_get( &ctx->tbl, ctx->used );

        bitvec_unset( &ctx->ids, ctx->used );
        dynarr_del( &ctx->tbl, ctx->used );
        ctx->used = -1;
        hclose( h );
    }

    if( bitvec_get( &ctx->ids, id ) == 1 )
    {
        // current coroutine should not release self handle
        if( dynarr_get( &ctx->tbl, id ) != -1 ){
            ctx->used = id;
        }
        // release context-id
        else {
            bitvec_unset( &ctx->ids, id );
        }
    }
}

