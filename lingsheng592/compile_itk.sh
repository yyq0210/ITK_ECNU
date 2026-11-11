#!/bin/bash
# Build Lingsheng mixed-precision ITK: static, 592 threads, float accumulation ON, gold linker.
# Preferred source: release/ITK-5.4.0-Huawei-Kunpeng.tar.gz (changes already baked in).
# Compile switch: -DITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS=ON
# /usr/bin/cmake only. Do not reconfigure an existing build tree unless ITK_FORCE_REBUILD=1.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
mkdir -p "$GOLD_DIR"
ln -sfn /usr/bin/ld.gold "$GOLD_DIR/ld"

source_baked() {
  [[ -f "$ITK_SRC/CMakeLists.txt" ]] \
    && grep -q 'ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS' "$ITK_SRC/CMakeLists.txt" \
    && grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_SRC/Modules/Filtering/Smoothing/include/itkMeanImageFilter.h" \
    && grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_SRC/Modules/Filtering/DistanceMap/include/itkDirectedHausdorffDistanceImageFilter.h"
}

install_current() {
  local conf="$ITK_INC/itkConfigure.h"
  local mean="$ITK_INC/itkMeanImageFilter.h"
  local haus="$ITK_INC/itkDirectedHausdorffDistanceImageFilter.h"
  [[ -f "$conf" && -f "$mean" && -f "$haus" ]] \
    && grep -q '#define ITK_DEFAULT_MAX_THREADS 592' "$conf" \
    && grep -q '#define ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS' "$conf" \
    && grep -q 'ITK_ACCUMULATION_TYPE' "$mean" \
    && grep -q 'ITK_ACCUMULATION_TYPE' "$haus"
}

if [[ "${ITK_FORCE_REBUILD:-0}" != "1" ]] && install_current; then
  echo "ALREADY_CURRENT $ITK_INSTALL"
  exit 0
fi

if [[ ! -f "$ITK_SRC/CMakeLists.txt" ]]; then
  echo "extract $ITK_TARBALL -> $(dirname "$ITK_SRC")"
  mkdir -p "$(dirname "$ITK_SRC")"
  # Remove partial extract of the expected leaf dir.
  rm -rf "$ITK_SRC"
  tar -xzf "$ITK_TARBALL" -C "$(dirname "$ITK_SRC")"
  # Huawei package top-level is ITK-5.4.0-Huawei-Kunpeng; allow rename if needed.
  if [[ ! -f "$ITK_SRC/CMakeLists.txt" ]]; then
    top="$(tar -tzf "$ITK_TARBALL" | head -1 | cut -d/ -f1)"
    if [[ -n "$top" && -f "$(dirname "$ITK_SRC")/$top/CMakeLists.txt" && "$(dirname "$ITK_SRC")/$top" != "$ITK_SRC" ]]; then
      mv "$(dirname "$ITK_SRC")/$top" "$ITK_SRC"
    fi
  fi
  test -f "$ITK_SRC/CMakeLists.txt"
fi

export ITK_SRC ITK_INC ITK_BUILD
if source_baked; then
  echo "SOURCE_BAKED $ITK_SRC (no external patch)"
else
  echo "SOURCE_NEEDS_PATCH $ITK_SRC — applying apply_float_accum.py (legacy tarball fallback)"
  python3 "$HERE/apply_float_accum.py"
fi

if [[ "${ITK_FORCE_REBUILD:-0}" == "1" && -d "$ITK_BUILD" ]]; then
  echo "ITK_FORCE_REBUILD=1 wipe $ITK_BUILD"
  rm -rf "$ITK_BUILD"
fi

if [[ ! -f "$ITK_BUILD/CMakeCache.txt" ]]; then
  "$CMAKE" -S "$ITK_SRC" -B "$ITK_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$ITK_INSTALL" \
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

# Header-only filters: ensure install tree matches baked source when build tree was reused.
if source_baked; then
  python3 - <<'PY'
import os, shutil
from pathlib import Path
src = Path(os.environ["ITK_SRC"])
inc = Path(os.environ["ITK_INC"])
pairs = [
  "Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h",
  "Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.hxx",
  "Modules/Filtering/Smoothing/include/itkMeanImageFilter.h",
  "Modules/Filtering/Smoothing/include/itkDiscreteGaussianImageFilter.h",
  "Modules/Filtering/Smoothing/include/itkBoxUtilities.h",
  "Modules/Filtering/AnisotropicSmoothing/include/itkCurvatureAnisotropicDiffusionImageFilter.h",
  "Modules/Filtering/AnisotropicSmoothing/include/itkGradientAnisotropicDiffusionImageFilter.h",
  "Modules/Filtering/DistanceMap/include/itkDirectedHausdorffDistanceImageFilter.h",
  "Modules/Filtering/DistanceMap/include/itkHausdorffDistanceImageFilter.h",
]
for rel in pairs:
  s = src / rel
  d = inc / Path(rel).name
  if s.exists() and inc.is_dir():
    d.write_text(s.read_text(encoding="utf-8"), encoding="utf-8")
    print("sync", d.name)
PY
else
  python3 "$HERE/apply_float_accum.py"
fi

test -f "$ITK_INC/itkConfigure.h"
grep -q 'ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS' "$ITK_INC/itkConfigure.h"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_INC/itkMeanImageFilter.h"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_INC/itkDirectedHausdorffDistanceImageFilter.h"
echo COMPILE_ITK_OK
