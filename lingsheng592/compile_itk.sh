#!/bin/bash
# Build the current Lingsheng ITK: static, 592 threads, float accumulation ON, gold linker.
# /usr/bin/cmake only. Do not reconfigure an existing build tree.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
mkdir -p "$GOLD_DIR"
ln -sfn /usr/bin/ld.gold "$GOLD_DIR/ld"

conf="$ITK_INC/itkConfigure.h"
mean="$ITK_INC/itkMeanImageFilter.h"
if [[ -f "$conf" && -f "$mean" ]] \
  && grep -q '#define ITK_DEFAULT_MAX_THREADS 592' "$conf" \
  && grep -q '#define ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS' "$conf" \
  && grep -q 'ITK_ACCUMULATION_TYPE' "$mean"; then
  echo "ALREADY_CURRENT $ITK_INSTALL"
  exit 0
fi

if [[ ! -f "$ITK_SRC/CMakeLists.txt" ]]; then
  echo "extract $ITK_TARBALL"
  mkdir -p "$(dirname "$ITK_SRC")"
  tar -xzf "$ITK_TARBALL" -C "$(dirname "$ITK_SRC")"
fi

export ITK_SRC ITK_INC ITK_BUILD
python3 "$HERE/apply_float_accum.py"

if [[ ! -f "$ITK_BUILD/CMakeCache.txt" ]]; then
  "$CMAKE" -S "$ITK_SRC" -B "$ITK_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTING=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DITK_DEFAULT_MAX_THREADS=592 \
    -DITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS=ON \
    -DCMAKE_C_COMPILER="$CC" \
    -DCMAKE_CXX_COMPILER="$CXX" \
    -DCMAKE_C_FLAGS="$FLAGS" \
    -DCMAKE_CXX_FLAGS="$FLAGS" \
    -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \
    -DCMAKE_MODULE_LINKER_FLAGS="$LDFLAGS" \
    -DCMAKE_STATIC_LINKER_FLAGS="$LDFLAGS"
  bash "$HERE/fix_generated_headers.sh"
else
  echo "keep existing build tree $ITK_BUILD"
fi

"$CMAKE" --build "$ITK_BUILD" -j "$(nproc)"
"$CMAKE" --install "$ITK_BUILD"
python3 "$HERE/apply_float_accum.py"
echo COMPILE_ITK_OK
