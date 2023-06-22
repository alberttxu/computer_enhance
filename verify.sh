#!/bin/sh
set -xe

listings="
listing37 \
listing38 \
listing39 \
listing40 \
listing41_nojumps \
listing43 \
listing44 \
listing45 \
listing46 \
listing48 \
listing51 \
"

sh build.sh

for asm in ${listings}; do
   nasm ${asm}.nasm
   ./decoder ${asm} > tempfile.nasm
   nasm tempfile.nasm
   diff tempfile ${asm}
done
