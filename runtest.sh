#!/bin/sh
set -xe

nasm test.nasm && ./build.sh && ./decoder test
