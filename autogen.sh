#!/bin/sh

set -e

#
# check arguments for debug build
#
for arg in $@
do
    case $arg in
        "debug" ) DEBUG="--enable-debug" ;;
        "valgrind" ) VALGRIND="--enable-valgrind" ;;
    esac
done

set -x

CURRENT_DIR=$PWD

git submodule update -i

#
# build libdill
#
cd deps/libdill
./autogen.sh
./configure --disable-shared --disable-threads $DEBUG $VALGRIND
make
make check


#
# generate configure script
#
cd $CURRENT_DIR
autoreconf -ivf
./configure $DEBUG
