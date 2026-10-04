#!/bin/bash
# Lingsheng paths. Do not put passwords in this file.
YYQ="${YYQ:-/home/share/nsls_yyq/yyq}"
ROOT="${ROOT:-$YYQ/ITK_ECNU}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

ITK_TARBALL="${ITK_TARBALL:-$YYQ/InsightToolkit-5.4.0.tar.gz}"
ITK_SRC="${ITK_SRC:-/tmp/itk-src/InsightToolkit-5.4.0}"
ITK_BUILD="${ITK_BUILD:-/tmp/itk-static}"
ITK_INSTALL="${ITK_INSTALL:-$YYQ/ITK-5.4.0-static/install}"
ITK_INC="${ITK_INC:-$ITK_INSTALL/include/ITK-5.4}"
ITK_DIR="${ITK_DIR:-$ITK_INSTALL/lib/cmake/ITK-5.4}"

CMAKE="${CMAKE:-/usr/bin/cmake}"
CC="${CC:-/usr/bin/gcc}"
CXX="${CXX:-/usr/bin/g++}"
GOLD_DIR="${GOLD_DIR:-$YYQ/ld-gold}"
FLAGS="${FLAGS:--B$GOLD_DIR -fno-use-linker-plugin -O3 -march=armv8.2-a+crypto}"
LDFLAGS="${LDFLAGS:--B$GOLD_DIR -fno-use-linker-plugin}"

if [[ -d "$ROOT/test/src" ]]; then
  BENCH_SRC="${BENCH_SRC:-$ROOT/test/src}"
  BENCH_BUILD="${BENCH_BUILD:-$ROOT/test/build}"
  DATA="${DATA:-$ROOT/test/data}"
elif [[ -d "$ROOT/送测/src" ]]; then
  BENCH_SRC="${BENCH_SRC:-$ROOT/送测/src}"
  BENCH_BUILD="${BENCH_BUILD:-$ROOT/送测/build}"
  DATA="${DATA:-$ROOT/送测/data}"
else
  BENCH_SRC="${BENCH_SRC:-$ROOT/test/src}"
  BENCH_BUILD="${BENCH_BUILD:-$ROOT/test/build}"
  DATA="${DATA:-$ROOT/test/data}"
fi

IMG="${IMG:-$DATA/BrainProtonDensity1024.png}"
PIN="${PIN:-$YYQ/pinpreload.so}"
THREADS="${THREADS:-592}"
QUALIFIED="${QUALIFIED:-$HERE/qualified_functions.tsv}"
COMPUTE_CPUS="${COMPUTE_CPUS:-0-36,38-74,76-112,114-150,152-188,190-226,228-264,266-302,304-340,342-378,380-416,418-454,456-492,494-530,532-568,570-606}"
