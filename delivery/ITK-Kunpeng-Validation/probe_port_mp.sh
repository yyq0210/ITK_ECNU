#!/bin/bash
set -u
ITK=/home/pub/yyq/ITK-5.4.0
echo "=== CMAKE SWITCH ==="
grep -n -A8 'ITK_ENABLE_MIXED_PRECISION' "$ITK/CMakeLists.txt" | head -20
echo "=== CONFIGURE H ==="
grep -n 'FLOAT_ACCUMULATION\|MIXED\|FLOAT_SPACE' "$ITK/build-yyq/Modules/Core/Common/itkConfigure.h" | head
echo "=== KUNPENG HEADER ==="
sed -n '1,40p' "$ITK/Modules/Core/Common/include/itkKunpengPrecision.h"
echo "=== MEAN ==="
grep -n 'InputRealType\|KunpengPixelAccum\|NumericTraits' "$ITK/Modules/Filtering/Smoothing/include/itkMeanImageFilter.h" | head -15
echo "=== DISCRETE GAUSSIAN ==="
grep -n 'RealOutputPixelType\|KunpengPixelAccum' "$ITK/Modules/Filtering/Smoothing/include/itkDiscreteGaussianImageFilter.h" | head -10
echo "=== BOX ==="
grep -n 'AccPixType\|KunpengPixelAccum' "$ITK/Modules/Filtering/Smoothing/include/itkBoxUtilities.h" | head -15
echo "=== BILATERAL H ==="
grep -n 'OutputPixelRealType\|KunpengPixelAccum' "$ITK/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h" | head -15
echo "=== FD PixelRealType ==="
grep -n 'PixelRealType' "$ITK/Modules/Core/FiniteDifference/include/itkFiniteDifferenceFunction.h" | head -10
echo "=== CAD GenerateData ==="
grep -n 'GenerateData\|ITK_USE_FLOAT_ACCUMULATION\|DoubleSelf\|kunpeng' "$ITK/Modules/Filtering/AnisotropicSmoothing/include/itkCurvatureAnisotropicDiffusionImageFilter.h" | head -20
echo "=== GAD GenerateData ==="
grep -n 'GenerateData\|ITK_USE_FLOAT_ACCUMULATION\|DoubleSelf\|kunpeng' "$ITK/Modules/Filtering/AnisotropicSmoothing/include/itkGradientAnisotropicDiffusionImageFilter.h" | head -20
echo "=== CURVATURE FLOW ==="
grep -n 'KunpengPixelAccum\|ITK_USE_FLOAT_ACCUMULATION\|GenerateData' "$ITK/Modules/Filtering/CurvatureFlow/include/itkCurvatureFlowImageFilter.h" | head -15
echo "=== MINMAX CURV ==="
grep -n 'KunpengPixelAccum\|ITK_USE_FLOAT_ACCUMULATION' "$ITK/Modules/Filtering/CurvatureFlow/include/itkMinMaxCurvatureFlowImageFilter.h" 2>/dev/null | head
echo "=== march in cache ==="
grep -E 'CMAKE_CXX_FLAGS:|CMAKE_C_FLAGS:|ITK_ENABLE_MIXED' "$ITK/build-yyq/CMakeCache.txt" | grep -v ADVANCED
