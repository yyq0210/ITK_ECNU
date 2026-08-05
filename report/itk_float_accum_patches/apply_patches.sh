#!/usr/bin/env bash
# 将 ITK Filter 内部累加从 NumericTraits::RealType 改为 FloatType（仅 float 像素路径）
set -euo pipefail

ITK_SRC="${ITK_SRC:-/home/pub/yyq/ITK-5.4.0}"
MARK="yyq: float accum"

patch_line() {
  local file="$1"
  local from="$2"
  local to="$3"
  if grep -q "$MARK" "$file" 2>/dev/null; then
    echo "skip (already patched): $file"
    return 0
  fi
  if ! grep -q "$from" "$file"; then
    echo "WARN: pattern not found in $file"
    return 1
  fi
  sed -i "s#${from}#${to} /* ${MARK} */#" "$file"
  echo "patched: $file"
}

echo "=== ITK float accum patches ==="
echo "ITK_SRC=$ITK_SRC"

patch_line \
  "$ITK_SRC/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h" \
  'using OutputPixelRealType = typename NumericTraits<OutputPixelType>::RealType;' \
  'using OutputPixelRealType = typename NumericTraits<OutputPixelType>::FloatType;'

patch_line \
  "$ITK_SRC/Modules/Filtering/Smoothing/include/itkMeanImageFilter.h" \
  'using InputRealType = typename NumericTraits<InputPixelType>::RealType;' \
  'using InputRealType = typename NumericTraits<InputPixelType>::FloatType;'

patch_line \
  "$ITK_SRC/Modules/Filtering/Smoothing/include/itkDiscreteGaussianImageFilter.h" \
  'using RealOutputPixelType = typename NumericTraits<OutputPixelType>::RealType;' \
  'using RealOutputPixelType = typename NumericTraits<OutputPixelType>::FloatType;'

echo "=== done ==="
