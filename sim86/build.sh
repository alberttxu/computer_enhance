#!/bin/sh
set -e

WARNINGS="-Wall -Wextra -Wstrict-prototypes"

cc ${WARNINGS} -o decoder -g3 -O0 decoder.c

