#!/usr/bin/env bash
# 扫频 TopN 算子 float accum 扩展（补丁 3）
# - CAD/GAD：itkFiniteDifferenceFunction.h PixelRealType
# - BoxMean 等：itkBoxUtilities.h AccPixType
set -euo pipefail

ITK_SRC="${ITK_SRC:-/home/pub/yyq/ITK-5.4.0}"
MARK="yyq: topn float accum"
FD="$ITK_SRC/Modules/Core/FiniteDifference/include/itkFiniteDifferenceFunction.h"
FDHXX="$ITK_SRC/Modules/Core/FiniteDifference/include/itkFiniteDifferenceImageFilter.hxx"
BOX="$ITK_SRC/Modules/Filtering/Smoothing/include/itkBoxUtilities.h"

if grep -q "$MARK" "$FD" 2>/dev/null && grep -q "$MARK" "$FDHXX" 2>/dev/null; then
  echo "skip (already patched): topn float accum"
  exit 0
fi

patch() {
  local file="$1" from="$2" to="$3"
  if ! grep -qF "$from" "$file"; then
    echo "WARN: pattern not found in $file:"
    echo "  $from"
    exit 1
  fi
  local from_esc to_esc
  from_esc=$(printf '%s' "$from" | sed 's/[[\.*^$]/\\&/g')
  to_esc=$(printf '%s' "$to" | sed 's/[\\&]/\\&/g')
  sed -i "s#${from_esc}#${to_esc} /* ${MARK} */#" "$file"
}

echo "=== ITK topn float accum patches ==="
echo "ITK_SRC=$ITK_SRC"

if ! grep -qF '#include "itkNumericTraits.h"' "$FD"; then
  sed -i '/#include "itkVector.h"/a #include "itkNumericTraits.h"' "$FD"
  echo "added itkNumericTraits.h include to $FD"
fi

if grep -qF '  using PixelRealType = double;' "$FD"; then
  patch "$FD" \
    '  using PixelRealType = double;' \
    '  using PixelRealType = typename NumericTraits<PixelType>::FloatType;'
  echo "patched: $FD"
elif grep -q "$MARK" "$FD"; then
  echo "skip (already patched): $FD"
else
  echo "WARN: unexpected PixelRealType state in $FD"
  exit 1
fi

if ! grep -q "$MARK" "$FDHXX" 2>/dev/null; then
  patch "$FDHXX" \
    '  double coeffs[ImageDimension];' \
    '  typename FiniteDifferenceFunctionType::PixelRealType coeffs[ImageDimension];'
  patch "$FDHXX" \
    '      coeffs[i] = 1.0 / static_cast<double>(spacing[i]);' \
    '      coeffs[i] = static_cast<typename FiniteDifferenceFunctionType::PixelRealType>(1.0 / static_cast<double>(spacing[i]));'
  echo "patched: $FDHXX"
fi

if grep -qF 'using AccPixType = typename NumericTraits<OutputPixelType>::RealType;' "$BOX"; then
  sed -i "s#using AccPixType = typename NumericTraits<OutputPixelType>::RealType;#using AccPixType = typename NumericTraits<OutputPixelType>::FloatType; /* ${MARK} */#g" "$BOX"
  echo "patched: $BOX"
elif grep -q "$MARK" "$BOX"; then
  echo "skip (already patched): $BOX"
else
  echo "WARN: unexpected AccPixType state in $BOX"
  exit 1
fi

echo "=== done ==="
