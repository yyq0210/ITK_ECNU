#!/usr/bin/env bash
# 还原 Bilateral full float 补丁
set -euo pipefail

ITK_SRC="${ITK_SRC:-/home/pub/yyq/ITK-5.4.0}"
MARK="yyq: bilateral full float"
H="$ITK_SRC/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h"
X="$ITK_SRC/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.hxx"

revert_file() {
  local f="$1"
  if [[ ! -f "$f" ]] || ! grep -q "$MARK" "$f"; then
    echo "skip (not patched): $f"
    return 0
  fi
  sed -i "s/ \/* ${MARK} *\//;/g" "$f"

  sed -i 's/Neighborhood<OutputPixelRealType, Self::ImageDimension>/Neighborhood<double, Self::ImageDimension>/g' "$f"
  sed -i 's/Image<OutputPixelRealType, Self::ImageDimension>/Image<double, Self::ImageDimension>/g' "$f"
  sed -i 's/OutputPixelRealType m_DynamicRange{}/double              m_DynamicRange{}/g' "$f"
  sed -i 's/OutputPixelRealType m_DynamicRangeUsed{}/double              m_DynamicRangeUsed{}/g' "$f"
  sed -i 's/std::vector<OutputPixelRealType> m_RangeGaussianTable{}/std::vector<double> m_RangeGaussianTable{}/g' "$f"

  sed -i 's/OutputPixelRealType                    norm = 0.0;/double                                 norm = 0.0;/g' "$f"
  sed -i 's/OutputPixelRealType rangeVariance = static_cast<OutputPixelRealType>(m_RangeSigma \* m_RangeSigma);/double rangeVariance = m_RangeSigma * m_RangeSigma;/g' "$f"
  sed -i 's/OutputPixelRealType rangeGaussianDenom;/double rangeGaussianDenom;/g' "$f"
  sed -i 's/rangeGaussianDenom = static_cast<OutputPixelRealType>(m_RangeSigma) \* static_cast<OutputPixelRealType>(std::sqrt(2.0 \* itk::Math::pi));/rangeGaussianDenom = m_RangeSigma * std::sqrt(2.0 * itk::Math::pi);/g' "$f"
  sed -i 's/OutputPixelRealType tableDelta;/double tableDelta;/g' "$f"
  sed -i 's/OutputPixelRealType v;/double v;/g' "$f"
  sed -i 's/m_DynamicRange = static_cast<OutputPixelRealType>(statistics->GetMaximum()) - static_cast<OutputPixelRealType>(statistics->GetMinimum());/m_DynamicRange = (static_cast<double>(statistics->GetMaximum()) - static_cast<double>(statistics->GetMinimum()));/g' "$f"
  sed -i 's/m_DynamicRangeUsed = static_cast<OutputPixelRealType>(m_RangeMu \* m_RangeSigma);/m_DynamicRangeUsed = m_RangeMu * m_RangeSigma;/g' "$f"
  sed -i 's/tableDelta = m_DynamicRangeUsed \/ static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples);/tableDelta = m_DynamicRangeUsed \/ static_cast<double>(m_NumberOfRangeGaussianSamples);/g' "$f"
  sed -i 's/const OutputPixelRealType            rangeDistanceThreshold = m_DynamicRangeUsed;/const double                         rangeDistanceThreshold = m_DynamicRangeUsed;/g' "$f"
  sed -i 's/const OutputPixelRealType distanceToTableIndex = static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples) \/ m_DynamicRangeUsed;/const double distanceToTableIndex = static_cast<double>(m_NumberOfRangeGaussianSamples) \/ m_DynamicRangeUsed;/g' "$f"

  echo "reverted: $f"
}

revert_file "$H"
revert_file "$X"
echo "=== reverted bilateral full float ==="
