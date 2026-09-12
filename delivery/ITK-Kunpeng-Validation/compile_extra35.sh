#!/bin/bash
set -euo pipefail
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin
unset CXXFLAGS CFLAGS CPATH
DEMO=/home/pub/yyq/itk_hybrid_precision_demo
ITK=/home/pub/yyq/ITK-5.4.0/build-yyq
cd "$DEMO"
env -i HOME="$HOME" PATH="$PATH" \
  cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=/home/pub/gjj/gcc10/bin/gcc \
    -DCMAKE_CXX_COMPILER=/home/pub/gjj/gcc10/bin/g++ \
    -DCMAKE_CXX_FLAGS='-O3 -march=armv8.2-a+crypto' \
    -DITK_DIR="$ITK"
cmake --build build --target precision_extra35_bench -j8
echo BUILD_OK
./build/precision_extra35_bench --list
