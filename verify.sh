#!/bin/sh
set -xe

listings="
listing37 \
listing38 \
listing39 \
listing40 \
listing41_nojumps \
"

sh build.sh

for asm in ${listings}; do
   nasm ${asm}.nasm
   ./decoder ${asm} > tempfile.nasm
   nasm tempfile.nasm
   diff tempfile ${asm}
done