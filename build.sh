#!/bin/sh
set -xe

WARNINGS="-Wall -Wextra -Wstrict-prototypes"

cc ${WARNINGS} -o decoder -g3 -O0 decoder.c

