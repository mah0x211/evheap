#!/bin/sh

set -e
set -x

CURRENT_DIR=$PWD

git submodule update -i

#
# build libdill
#
cd deps/libdill
autoreconf -ivf
./configure --disable-shared --disable-threads
make
make check


#
# generate configure script
#
cd $CURRENT_DIR
autoreconf -ivf
