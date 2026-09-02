#!/usr/bin/env bash
# 将 CAD/GAD 有限差分更新改回 double（图像仍可用 float），降低迭代误差。
# 不改动 Mean/Bilateral/BoxMean 等滤波 float 累加。
set -euo pipefail

ITK_SRC="${ITK_SRC:-/home/pub/yyq/ITK-5.4.0}"
BUILD="${BUILD:-${ITK_SRC}/build-yyq}"
DEMO="${DEMO:-/home/pub/yyq/itk_hybrid_precision_demo}"
DATA="${DATA:-${ITK_SRC}/Examples/Data}"
THREADS="${THREADS:-96}"
REPORT="${REPORT:-/tmp/cad_gad_double_fd_$(date +%F_%H%M%S).log}"

FD="$ITK_SRC/Modules/Core/FiniteDifference/include/itkFiniteDifferenceFunction.h"
FDHXX="$ITK_SRC/Modules/Core/FiniteDifference/include/itkFiniteDifferenceImageFilter.hxx"
BOX="$ITK_SRC/Modules/Filtering/Smoothing/include/itkBoxUtilities.h"

export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS="$THREADS"
export PATH="/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:${PATH:-}"

exec > >(tee "$REPORT") 2>&1

echo "=== CAD/GAD PixelRealType -> double ==="
echo "date=$(date -Iseconds) host=$(hostname) report=$REPORT"
echo "ITK_SRC=$ITK_SRC BUILD=$BUILD"

[[ -f "$FD" && -f "$FDHXX" ]] || { echo "ERROR: missing FiniteDifference headers"; exit 1; }

echo
echo "######## 0. before ########"
grep -n "PixelRealType" "$FD" | head -8 || true
grep -n "coeffs" "$FDHXX" | head -8 || true
if [[ -f "$BOX" ]]; then
  echo "BoxMean AccPixType (should stay FloatType):"
  grep -n "AccPixType" "$BOX" | head -6 || true
fi

python3 - "$FD" "$FDHXX" <<'PY'
import re, sys
fd, fdhxx = sys.argv[1], sys.argv[2]

def load(p):
    with open(p, encoding="utf-8") as f:
        return f.read()

def save(p, t):
    with open(p, "w", encoding="utf-8") as f:
        f.write(t)

text = load(fd)
text = re.sub(r" /\* yyq: [^*]* \*/", "", text)
old = text
# float 累加版 -> upstream double
text = text.replace(
    "using PixelRealType = typename NumericTraits<PixelType>::FloatType;",
    "using PixelRealType = double;",
)
if text == old and "using PixelRealType = double;" not in text:
    raise SystemExit("unexpected PixelRealType in " + fd)
save(fd, text)
print("updated:", fd)
print("  PixelRealType now:",
      "double" if "using PixelRealType = double;" in text else "UNKNOWN")

hxx = load(fdhxx)
hxx = re.sub(r" /\* yyq: [^*]* \*/", "", hxx)
hxx2 = hxx.replace(
    "typename FiniteDifferenceFunctionType::PixelRealType coeffs[ImageDimension];",
    "double coeffs[ImageDimension];",
)
hxx2 = hxx2.replace(
    "coeffs[i] = static_cast<typename FiniteDifferenceFunctionType::PixelRealType>(1.0 / static_cast<double>(spacing[i]));",
    "coeffs[i] = 1.0 / static_cast<double>(spacing[i]);",
)
save(fdhxx, hxx2)
print("updated:", fdhxx)
if "double coeffs[ImageDimension];" in hxx2:
    print("  coeffs type: double")
else:
    print("  WARN: coeffs pattern not in expected form")
PY

echo
echo "######## 1. after ########"
grep -n "using PixelRealType" "$FD"
grep -n "coeffs" "$FDHXX" | head -8
if [[ -f "$BOX" ]]; then
  grep -n "AccPixType" "$BOX" | head -6
fi

echo
echo "######## 2. rebuild ITK FiniteDifference + AnisotropicSmoothing ########"
cd "$BUILD"
cmake --build . --target ITKCommon ITKAnisotropicSmoothing-all -j"${THREADS}"

echo
echo "######## 3. rebuild precision_diffusion_bench ########"
if [[ ! -d "$DEMO" ]]; then
  echo "WARN: demo dir missing: $DEMO"
  exit 0
fi
mkdir -p "$DEMO/build"
cd "$DEMO/build"
cmake .. -DCMAKE_CXX_COMPILER=/home/pub/gjj/gcc10/bin/g++ -DITK_DIR="$BUILD"
cmake --build . --target precision_diffusion_bench -j8

IMG="$DATA/BrainProtonDensitySlice.png"
if [[ ! -f "$IMG" ]]; then
  IMG="$DATA/BrainProtonDensitySlice256x256.png"
fi
echo
echo "######## 4. run bench: $IMG ########"
taskset -c 0-95 ./precision_diffusion_bench "$IMG" 50 0.125 3 5
echo "=== done report=$REPORT ==="
