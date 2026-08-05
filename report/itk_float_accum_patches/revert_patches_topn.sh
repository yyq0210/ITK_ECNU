#!/usr/bin/env bash
# 还原 topn float accum 补丁
set -euo pipefail

ITK_SRC="${ITK_SRC:-/home/pub/yyq/ITK-5.4.0}"
MARK="yyq: topn float accum"
FD="$ITK_SRC/Modules/Core/FiniteDifference/include/itkFiniteDifferenceFunction.h"
FDHXX="$ITK_SRC/Modules/Core/FiniteDifference/include/itkFiniteDifferenceImageFilter.hxx"
BOX="$ITK_SRC/Modules/Filtering/Smoothing/include/itkBoxUtilities.h"

revert_file() {
  local f="$1"
  if [[ ! -f "$f" ]] || ! grep -q "$MARK" "$f"; then
    echo "skip (not patched): $f"
    return 0
  fi
  sed -i "s/ \/* ${MARK} *\//;/g" "$f"
  sed -i 's/using PixelRealType = typename NumericTraits<PixelType>::FloatType;/using PixelRealType = double;/g' "$f"
  sed -i 's/typename FiniteDifferenceFunctionType::PixelRealType coeffs\[ImageDimension\];/double coeffs[ImageDimension];/g' "$f"
  sed -i 's/coeffs\[i\] = static_cast<typename FiniteDifferenceFunctionType::PixelRealType>(1.0 \/ static_cast<double>(spacing\[i\]));/coeffs[i] = 1.0 \/ static_cast<double>(spacing[i]);/g' "$f"
  sed -i 's/using AccPixType = typename NumericTraits<OutputPixelType>::FloatType;/using AccPixType = typename NumericTraits<OutputPixelType>::RealType;/g' "$f"
  echo "reverted: $f"
}

revert_file "$FD"
revert_file "$FDHXX"
revert_file "$BOX"
echo "=== reverted topn float accum ==="
