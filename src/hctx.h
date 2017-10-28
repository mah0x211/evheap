/*

  Copyright (C) 2017 Masatoshi Teruya

  evheap/src/hctx.h
  Created by Masatoshi Teruya on 17/10/27

*/

#ifndef hctx_h
#define hctx_h

#include "libdill.h"
#include <stdio.h>
#include "bitvec.h"
#include "dynarr.h"


typedef struct {
    dynarr_t tbl;
    bitvec_t ids;
    int used;
} hctx_t;

#define hctx_initializer {      \
    .tbl = dynarr_initializer,  \
    .ids = {                    \
        .nbit = 0,              \
        .nvec = 0,              \
        .vec = NULL             \
    },                          \
    .used = -1                  \
}

int hctx_alloc( hctx_t *ctx );
void hctx_dealloc( hctx_t *ctx );
int hctx_get( hctx_t *ctx );
int hctx_set( hctx_t *ctx, int id, int h );
void hctx_del( hctx_t *ctx, int id );

#define handle_close(ctx, id) hctx_del(ctx, id)

// return 0 on success, -1 on error
#define handle(ctx, f, ...) ({                                      \
    int rc_ = -1;                                                   \
    /* create new context-id */                                     \
    int id_ = hctx_get((ctx));                                      \
    if(id_ != -1){                                                  \
        /* run handler() */                                         \
        int h_ = go(f( (ctx), id_, __VA_ARGS__));                   \
        if(h_ != -1){                                               \
            /* associate handler-id with context-id */              \
            rc_ = hctx_set((ctx), id_, h_);                         \
            if(rc_ != 0){                                           \
                switch(errno){                                      \
                    /* handler already finished */                  \
                    case ENOENT:                                    \
                        errno = 0;                                  \
                        rc_ = 0;                                    \
                        break;                                      \
                    case ENOMEM:                                    \
                        break;                                      \
                    /* probably, bug in internal implementation */  \
                    /* EINVAL */                                    \
                    /* EEXIST */                                    \
                    default:                                        \
                        errno = ENOTRECOVERABLE;                    \
                        break;                                      \
                }                                                   \
                hctx_del((ctx), id_);                               \
                hclose(h_);                                         \
            }                                                       \
        }                                                           \
        /* failed to run handler() */                               \
        else {                                                      \
            hctx_del((ctx), id_);                                   \
        }                                                           \
    }                                                               \
    rc_;                                                            \
})


#endif /* hctx_h */

