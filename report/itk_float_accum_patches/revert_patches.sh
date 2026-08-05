#!/usr/bin/env bash
# 还原 float accum 补丁（恢复 RealType）
set -euo pipefail

ITK_SRC="${ITK_SRC:-/home/pub/yyq/ITK-5.4.0}"
MARK="yyq: float accum"

revert_file() {
  local f="$1"
  if [[ -f "$f" ]] && grep -q "$MARK" "$f"; then
    sed -i "s/ \/* ${MARK} *\//;/g" "$f"
    sed -i 's/NumericTraits<OutputPixelType>::FloatType;/NumericTraits<OutputPixelType>::RealType;/g' "$f"
    sed -i 's/NumericTraits<InputPixelType>::FloatType;/NumericTraits<InputPixelType>::RealType;/g' "$f"
    echo "reverted: $f"
  fi
}

revert_file "$ITK_SRC/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h"
revert_file "$ITK_SRC/Modules/Filtering/Smoothing/include/itkMeanImageFilter.h"
revert_file "$ITK_SRC/Modules/Filtering/Smoothing/include/itkDiscreteGaussianImageFilter.h"
echo "=== reverted ==="
