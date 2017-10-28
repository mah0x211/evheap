AM_CPPFLAGS = -I../deps/libdill -I../deps/bitvec @WARNINGS@
LDADD = ../deps/libdill/.libs/libdill.a
bin_PROGRAMS = evheap
include_HEADERS = evheap.h
evheap_SOURCES = main.c     \
                 sigch.c    \
                 dynarr.c   \
                 hctx.c     \
                 msgio.c    \
                 rawio.c    \
                 server.c
