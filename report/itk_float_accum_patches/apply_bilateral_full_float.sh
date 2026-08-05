#!/usr/bin/env bash
# Bilateral：域核 / 值域查表 / 内层循环阈值与索引 改为 OutputPixelRealType（float 像素时为 float）
# 需先应用 apply_patches.sh（OutputPixelRealType = FloatType）
set -euo pipefail

ITK_SRC="${ITK_SRC:-/home/pub/yyq/ITK-5.4.0}"
MARK="yyq: bilateral full float"
H="$ITK_SRC/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h"
X="$ITK_SRC/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.hxx"

if grep -q "$MARK" "$H" 2>/dev/null; then
  echo "skip (already patched): bilateral full float"
  exit 0
fi

if ! grep -q "yyq: float accum" "$H"; then
  echo "ERROR: run apply_patches.sh first (OutputPixelRealType must be FloatType)"
  exit 1
fi

patch() {
  local file="$1" from="$2" to="$3"
  if ! grep -qF "$from" "$file"; then
    echo "WARN: pattern not found in $file:"
    echo "  $from"
    exit 1
  fi
  sed -i "s#${from}#${to} /* ${MARK} */#" "$file"
}

echo "=== Bilateral full float patch ==="
echo "ITK_SRC=$ITK_SRC"

patch "$H" \
  'using KernelType = Neighborhood<double, Self::ImageDimension>;' \
  'using KernelType = Neighborhood<OutputPixelRealType, Self::ImageDimension>;'

patch "$H" \
  'using GaussianImageType = Image<double, Self::ImageDimension>;' \
  'using GaussianImageType = Image<OutputPixelRealType, Self::ImageDimension>;'

patch "$H" \
  '  double              m_DynamicRange{};' \
  '  OutputPixelRealType m_DynamicRange{};'

patch "$H" \
  '  double              m_DynamicRangeUsed{};' \
  '  OutputPixelRealType m_DynamicRangeUsed{};'

patch "$H" \
  '  std::vector<double> m_RangeGaussianTable{};' \
  '  std::vector<OutputPixelRealType> m_RangeGaussianTable{};'

patch "$X" \
  '  double                                 norm = 0.0;' \
  '  OutputPixelRealType                    norm = 0.0;'

patch "$X" \
  '  double rangeVariance = m_RangeSigma * m_RangeSigma;' \
  '  OutputPixelRealType rangeVariance = static_cast<OutputPixelRealType>(m_RangeSigma * m_RangeSigma);'

patch "$X" \
  '  double rangeGaussianDenom;' \
  '  OutputPixelRealType rangeGaussianDenom;'

patch "$X" \
  '  rangeGaussianDenom = m_RangeSigma * std::sqrt(2.0 * itk::Math::pi);' \
  '  rangeGaussianDenom = static_cast<OutputPixelRealType>(m_RangeSigma) * static_cast<OutputPixelRealType>(std::sqrt(2.0 * itk::Math::pi));'

patch "$X" \
  '  double tableDelta;' \
  '  OutputPixelRealType tableDelta;'

patch "$X" \
  '  double v;' \
  '  OutputPixelRealType v;'

patch "$X" \
  '  m_DynamicRange = (static_cast<double>(statistics->GetMaximum()) - static_cast<double>(statistics->GetMinimum()));' \
  '  m_DynamicRange = static_cast<OutputPixelRealType>(statistics->GetMaximum()) - static_cast<OutputPixelRealType>(statistics->GetMinimum());'

patch "$X" \
  '  m_DynamicRangeUsed = m_RangeMu * m_RangeSigma;' \
  '  m_DynamicRangeUsed = static_cast<OutputPixelRealType>(m_RangeMu * m_RangeSigma);'

patch "$X" \
  '  tableDelta = m_DynamicRangeUsed / static_cast<double>(m_NumberOfRangeGaussianSamples);' \
  '  tableDelta = m_DynamicRangeUsed / static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples);'

patch "$X" \
  '  const double                         rangeDistanceThreshold = m_DynamicRangeUsed;' \
  '  const OutputPixelRealType            rangeDistanceThreshold = m_DynamicRangeUsed;'

patch "$X" \
  '  const double distanceToTableIndex = static_cast<double>(m_NumberOfRangeGaussianSamples) / m_DynamicRangeUsed;' \
  '  const OutputPixelRealType distanceToTableIndex = static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples) / m_DynamicRangeUsed;'

echo "patched: $H"
echo "patched: $X"
echo "=== done ==="
