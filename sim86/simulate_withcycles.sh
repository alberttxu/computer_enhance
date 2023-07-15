#!/bin/sh

if [ -z $1 ]; then
   echo "need a nasm file as the first argument"
   exit 1
fi

set -xe

nasm -o test $1 && ./build.sh && ./decoder -exec -cycles test
