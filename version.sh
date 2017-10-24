#!/bin/sh

if [ -d .git ]; then
    VER=`git symbolic-ref -q --short HEAD || git describe --tags --exact-match`
else
    VER="unknown"
fi

printf '%s' "$VER"


