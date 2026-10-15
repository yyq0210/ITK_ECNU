# 灵昇 592 核：五轮均达标函数（217）

来源：作业 **1859213**（cn22968，`err_ok_repeat_20261011_205128`），对约 442 个「误差已 `<1e-5`」的能做混合精度接口连跑 **5** 轮。

达标条件（每轮同时满足）：

- 可判定的 `max_abs < 1e-5`（套件里仅 4 位小数的 `0.0000` 不算）
- `speedup = double_ms / float_ms > 1`

| 口径 | 数量 |
| --- | ---: |
| 各轮合格数 | 300, 280, 286, 284, 281 |
| 浮动区间 | [280, 300] |
| **五轮都达标（本文）** | **217** |
| 送测名单（区间上界，见 `qualified_functions.tsv`） | 300 |
| 至少一轮达标 | 350 |

本文是稳定交集，不是送测上界名单。纯名单见 `stable_every_round_217.txt`。

下表时间/误差取自第 1 轮 `measurements.csv`（五轮均达标，第 1 轮亦在内）；模块名来自 `mixed_precision_lingsheng592.csv`。

## 按模块汇总

| 模块 | 个数 |
| --- | ---: |
| Filtering/ImageIntensity | 40 |
| Filtering/ImageGrid | 22 |
| Filtering/Thresholding | 18 |
| Filtering/MathematicalMorphology | 13 |
| Filtering/FFT | 11 |
| Filtering/ImageFeature | 10 |
| Filtering/ImageStatistics | 10 |
| Segmentation/LevelSets | 10 |
| Filtering/BinaryMathematicalMorphology | 8 |
| Filtering/ImageGradient | 5 |
| Filtering/LabelMap | 5 |
| Filtering/Smoothing | 5 |
| Segmentation/RegionGrowing | 5 |
| Filtering/Convolution | 4 |
| Registration/Common | 4 |
| Registration/RegistrationMethodsv4 | 4 |
| Segmentation/LabelVoting | 4 |
| Filtering/DisplacementField | 3 |
| Filtering/DistanceMap | 3 |
| Filtering/ImageCompare | 3 |
| Filtering/ImageCompose | 3 |
| Filtering/ImageFilterBase | 3 |
| Registration/Metricsv4 | 3 |
| Registration/PDEDeformable | 3 |
| Segmentation/Watersheds | 3 |
| Filtering/AnisotropicSmoothing | 2 |
| Filtering/Path | 2 |
| Segmentation/Classifiers | 2 |
| Segmentation/ConnectedComponents | 2 |
| Filtering/AntiAlias | 1 |
| Filtering/Colormap | 1 |
| Filtering/CurvatureFlow | 1 |
| Filtering/ImageFrequency | 1 |
| Filtering/QuadEdgeMeshFiltering | 1 |
| Segmentation/DeformableMesh | 1 |
| Segmentation/Voronoi | 1 |
| **合计** | **217** |

## 分模块名单

### Filtering/ImageIntensity（40）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AbsImageFilter` | 7.8342 | 15.9576 | 2.0369 | 0 |
| `AcosImageFilter` | 7.5571 | 17.2223 | 2.2789 | 4.00e-08 |
| `AddImageFilter` | 7.5433 | 16.8429 | 2.2328 | 0 |
| `AsinImageFilter` | 7.5138 | 17.1703 | 2.2852 | 4.00e-08 |
| `Atan2ImageFilter` | 6.9943 | 16.3998 | 2.3447 | 2.00e-08 |
| `AtanImageFilter` | 7.3679 | 16.9952 | 2.3067 | 6.00e-08 |
| `BoundedReciprocalImageFilter` | 7.2592 | 16.3668 | 2.2546 | 1.00e-08 |
| `ClampImageFilter` | 6.9148 | 16.4559 | 2.3798 | 0 |
| `ComplexToImaginaryImageFilter` | 7.2312 | 7.4084 | 1.0245 | 0 |
| `ConstrainedValueAdditionImageFilter` | 6.804 | 17.0456 | 2.5052 | 0 |
| `ConstrainedValueDifferenceImageFilter` | 7.1246 | 14.2873 | 2.0053 | 0 |
| `CosImageFilter` | 7.9536 | 17.5782 | 2.2101 | 3.00e-08 |
| `DivideOrZeroOutImageFilter` | 6.9192 | 16.0499 | 2.3196 | 0 |
| `ExpNegativeImageFilter` | 6.7115 | 16.515 | 2.4607 | 1.00e-08 |
| `HistogramMatchingImageFilter` | 180.7535 | 216.2858 | 1.1966 | 0 |
| `IntensityWindowingImageFilter` | 6.9403 | 15.2332 | 2.1949 | 0 |
| `Log10ImageFilter` | 6.9489 | 15.7447 | 2.2658 | 1.20e-07 |
| `LogImageFilter` | 7.4046 | 17.1576 | 2.3172 | 2.40e-07 |
| `MagnitudeAndPhaseToComplexImageFilter` | 7.9406 | 8.8405 | 1.1133 | 0 |
| `MaskImageFilter` | 7.7031 | 18.5215 | 2.4044 | 0 |
| `MaskNegatedImageFilter` | 7.0081 | 16.7748 | 2.3936 | 0 |
| `MaximumImageFilter` | 7.1707 | 16.0882 | 2.2436 | 0 |
| `MinimumImageFilter` | 6.9372 | 16.0315 | 2.3109 | 0 |
| `MultiplyImageFilter` | 6.9874 | 16.2747 | 2.3292 | 0 |
| `NaryAddImageFilter` | 6.9324 | 14.0664 | 2.0291 | 0 |
| `NaryMaximumImageFilter` | 7.2856 | 13.9742 | 1.9181 | 0 |
| `NormalizeImageFilter` | 13.5778 | 23.1959 | 1.7084 | 6.00e-08 |
| `NormalizeToConstantImageFilter` | 13.2728 | 23.2018 | 1.7481 | 0 |
| `RescaleIntensityImageFilter` | 13.9096 | 32.7395 | 2.3537 | 3.00e-08 |
| `RoundImageFilter` | 6.8346 | 15.7601 | 2.3059 | 0 |
| `ShiftScaleImageFilter` | 6.8357 | 16.2783 | 2.3814 | 0 |
| `SinImageFilter` | 7.5388 | 17.0807 | 2.2657 | 3.00e-08 |
| `SqrtImageFilter` | 7.7663 | 17.4574 | 2.2478 | 4.70e-07 |
| `SquareImageFilter` | 7.1355 | 16.6465 | 2.3329 | 0 |
| `SubtractImageFilter` | 7.0473 | 16.2833 | 2.3106 | 0 |
| `SymmetricEigenAnalysisImageFilter` | 0.9084 | 0.9771 | 1.0756 | 0 |
| `TanImageFilter` | 7.7091 | 16.8701 | 2.1883 | 4.99e-06 |
| `TernaryAddImageFilter` | 7.0854 | 14.5329 | 2.0511 | 0 |
| `VectorMagnitudeImageFilter` | 14.0632 | 27.198 | 1.934 | 0 |
| `WeightedAddImageFilter` | 6.9868 | 16.7711 | 2.4004 | 0 |

### Filtering/ImageGrid（22）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinShrinkImageFilter` | 6.9133 | 14.6879 | 2.1246 | 0 |
| `BSplineDownsampleImageFilter` | 14.9541 | 23.0706 | 1.5428 | 0 |
| `BSplineUpsampleImageFilter` | 48.4136 | 82.5823 | 1.7058 | 0 |
| `ChangeInformationImageFilter` | 0.0502 | 7.1658 | 142.8564 | 0 |
| `ConstantPadImageFilter` | 6.8229 | 14.5971 | 2.1394 | 0 |
| `CropImageFilter` | 7.4721 | 16.9576 | 2.2695 | 0 |
| `CyclicShiftImageFilter` | 6.947 | 16.3834 | 2.3584 | 0 |
| `ExpandImageFilter` | 7.3345 | 17.3483 | 2.3653 | 0 |
| `FlipImageFilter` | 7.5336 | 17.7341 | 2.354 | 0 |
| `InterpolateImageFilter` | 38.5998 | 44.2292 | 1.1458 | 0 |
| `InterpolateImagePointsFilter` | 1.6089 | 1.6287 | 1.0123 | 0 |
| `MirrorPadImageFilter` | 7.2803 | 14.3499 | 1.9711 | 0 |
| `PasteImageFilter` | 7.1366 | 16.8438 | 2.3602 | 0 |
| `PermuteAxesImageFilter` | 7.2941 | 17.3973 | 2.3851 | 0 |
| `RegionOfInterestImageFilter` | 6.8308 | 15.7076 | 2.2995 | 0 |
| `ShrinkImageFilter` | 6.5712 | 14.592 | 2.2206 | 0 |
| `SliceBySliceImageFilter` | 2.839 | 3.3644 | 1.185 | 0 |
| `TileImageFilter` | 18.2301 | 26.3575 | 1.4458 | 0 |
| `WarpImageFilter` | 7.2958 | 18.3105 | 2.5097 | 7.93e-06 |
| `WarpVectorImageFilter` | 7.4325 | 7.5163 | 1.0113 | 0 |
| `WrapPadImageFilter` | 7.04 | 14.3139 | 2.0332 | 0 |
| `ZeroFluxNeumannPadImageFilter` | 6.8933 | 14.0799 | 2.0426 | 0 |

### Filtering/Thresholding（18）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryThresholdImageFilter` | 7.1174 | 16.2108 | 2.2776 | 0 |
| `BinaryThresholdProjectionImageFilter` | 7.0358 | 15.6322 | 2.2218 | 0 |
| `HistogramThresholdImageFilter` | 22.463 | 30.7687 | 1.3698 | 0 |
| `HuangThresholdImageFilter` | 22.0374 | 33.6994 | 1.5292 | 0 |
| `IntermodesThresholdImageFilter` | 21.4944 | 32.4523 | 1.5098 | 0 |
| `KappaSigmaThresholdImageFilter` | 34.4434 | 89.5824 | 2.6009 | 0 |
| `KittlerIllingworthThresholdImageFilter` | 21.4353 | 32.725 | 1.5267 | 0 |
| `LiThresholdImageFilter` | 21.5119 | 33.2726 | 1.5467 | 0 |
| `MaximumEntropyThresholdImageFilter` | 23.3795 | 35.4494 | 1.5163 | 0 |
| `MomentsThresholdImageFilter` | 21.2114 | 34.1042 | 1.6078 | 0 |
| `OtsuMultipleThresholdsImageFilter` | 108.9585 | 143.3883 | 1.316 | 0 |
| `OtsuThresholdImageFilter` | 21.6806 | 33.4036 | 1.5407 | 0 |
| `RenyiEntropyThresholdImageFilter` | 25.1329 | 36.0498 | 1.4344 | 0 |
| `ShanbhagThresholdImageFilter` | 22.4486 | 34.3179 | 1.5287 | 0 |
| `ThresholdImageFilter` | 6.8927 | 16.5328 | 2.3986 | 0 |
| `ThresholdLabelerImageFilter` | 6.9502 | 16.2926 | 2.3442 | 0 |
| `TriangleThresholdImageFilter` | 21.8577 | 32.8761 | 1.5041 | 0 |
| `YenThresholdImageFilter` | 21.6215 | 33.8276 | 1.5645 | 0 |

### Filtering/MathematicalMorphology（13）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BlackTopHatImageFilter` | 35.8003 | 51.6204 | 1.4419 | 0 |
| `GrayscaleConnectedClosingImageFilter` | 605.2005 | 703.5651 | 1.1625 | 0 |
| `GrayscaleConnectedOpeningImageFilter` | 861.6441 | 1046.9156 | 1.215 | 0 |
| `GrayscaleDilateImageFilter` | 7.4938 | 16.9133 | 2.257 | 0 |
| `GrayscaleErodeImageFilter` | 7.5453 | 17.0967 | 2.2659 | 0 |
| `GrayscaleFillholeImageFilter` | 493.2985 | 542.9275 | 1.1006 | 0 |
| `GrayscaleMorphologicalClosingImageFilter` | 29.924 | 44.2706 | 1.4794 | 0 |
| `GrayscaleMorphologicalOpeningImageFilter` | 30.3707 | 43.771 | 1.4412 | 0 |
| `MorphologicalGradientImageFilter` | 21.3635 | 33.1572 | 1.552 | 0 |
| `RankImageFilter` | 8.0787 | 19.646 | 2.4318 | 0 |
| `ReconstructionByErosionImageFilter` | 195.0468 | 203.3043 | 1.0423 | 0 |
| `RegionalMaximaImageFilter` | 135.6748 | 157.9303 | 1.164 | 0 |
| `WhiteTopHatImageFilter` | 36.5326 | 50.7511 | 1.3892 | 0 |

### Filtering/FFT（11）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `FFTPadImageFilter` | 7.0321 | 16.1258 | 2.2932 | 0 |
| `FFTShiftImageFilter` | 6.8499 | 16.5439 | 2.4152 | 0 |
| `FullToHalfHermitianImageFilter` | 7.9283 | 8.0696 | 1.0178 | 0 |
| `HalfToFullHermitianImageFilter` | 8.4063 | 8.8576 | 1.0537 | 0 |
| `VnlComplexToComplexFFTImageFilter` | 1.0662 | 1.126 | 1.0561 | 0 |
| `VnlForward1DFFTImageFilter` | 8.4193 | 9.4884 | 1.127 | 0 |
| `VnlForwardFFTImageFilter` | 87.822 | 110.6848 | 1.2603 | 0 |
| `VnlHalfHermitianToRealInverseFFTImageFilter` | 92.8893 | 106.3923 | 1.1454 | 0 |
| `VnlInverse1DFFTImageFilter` | 7.2494 | 7.2818 | 1.0045 | 0 |
| `VnlInverseFFTImageFilter` | 162.4298 | 201.8529 | 1.2427 | 0 |
| `VnlRealToHalfHermitianForwardFFTImageFilter` | 78.755 | 87.5729 | 1.112 | 0 |

### Filtering/ImageFeature（10）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `CannyEdgeDetectionImageFilter` | 61.8078 | 80.3149 | 1.2994 | 0 |
| `DerivativeImageFilter` | 7.9243 | 16.663 | 2.1028 | 0 |
| `DiscreteGaussianDerivativeImageFilter` | 33.7813 | 48.4713 | 1.4349 | 4.50e-07 |
| `Hessian3DToVesselnessMeasureImageFilter` | 6.9115 | 7.08 | 1.0244 | 0 |
| `HoughTransform2DLinesImageFilter` | 2.2741 | 3.05 | 1.3412 | 0 |
| `LaplacianImageFilter` | 7.4486 | 17.3513 | 2.3295 | 0 |
| `LaplacianRecursiveGaussianImageFilter` | 54.601 | 69.8979 | 1.2802 | 0 |
| `LaplacianSharpeningImageFilter` | 40.9032 | 52.1675 | 1.2754 | 7.23e-06 |
| `SimpleContourExtractorImageFilter` | 7.3827 | 16.5295 | 2.239 | 0 |
| `ZeroCrossingImageFilter` | 7.0139 | 16.4009 | 2.3383 | 0 |

### Filtering/ImageStatistics（10）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AccumulateImageFilter` | 19.9785 | 34.1718 | 1.7104 | 0 |
| `AdaptiveHistogramEqualizationImageFilter` | 17.0296 | 25.7202 | 1.5103 | 7.63e-06 |
| `MaximumProjectionImageFilter` | 6.6756 | 15.1959 | 2.2763 | 0 |
| `MeanProjectionImageFilter` | 6.6496 | 14.7256 | 2.2145 | 0 |
| `MedianProjectionImageFilter` | 7.2297 | 15.6579 | 2.1658 | 0 |
| `MinimumMaximumImageFilter` | 7.0304 | 14.0955 | 2.0049 | 0 |
| `MinimumProjectionImageFilter` | 6.6398 | 15.5503 | 2.342 | 0 |
| `StandardDeviationProjectionImageFilter` | 7.2195 | 15.4809 | 2.1443 | 0 |
| `StatisticsImageFilter` | 6.9104 | 14.09 | 2.039 | 0 |
| `SumProjectionImageFilter` | 6.7209 | 15.3194 | 2.2794 | 0 |

### Segmentation/LevelSets（10）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AnisotropicFourthOrderLevelSetImageFilter` | 7034.1766 | 7147.9776 | 1.0162 | 0 |
| `BinaryMaskToNarrowBandPointSetFilter` | 118.3124 | 123.3948 | 1.043 | 0 |
| `CurvesLevelSetImageFilter` | 207.1388 | 208.3584 | 1.0059 | 0 |
| `ExtensionVelocitiesImageFilter` | 1091.9225 | 1113.9787 | 1.0202 | 0 |
| `GeodesicActiveContourShapePriorLevelSetImageFilter` | 12.5291 | 12.6404 | 1.0089 | 0 |
| `ImplicitManifoldNormalVectorFilter` | 81.5871 | 82.3634 | 1.0095 | 0 |
| `LaplacianSegmentationLevelSetImageFilter` | 75.0735 | 83.4431 | 1.1115 | 0 |
| `NarrowBandCurvesLevelSetImageFilter` | 406.1503 | 411.1099 | 1.0122 | 0 |
| `NarrowBandThresholdSegmentationLevelSetImageFilter` | 253.617 | 264.4653 | 1.0428 | 0 |
| `ThresholdSegmentationLevelSetImageFilter` | 70.9765 | 91.2479 | 1.2856 | 0 |

### Filtering/BinaryMathematicalMorphology（8）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryClosingByReconstructionImageFilter` | 149.6833 | 179.846 | 1.2015 | 0 |
| `BinaryDilateImageFilter` | 69.9183 | 89.0048 | 1.273 | 0 |
| `BinaryMorphologicalClosingImageFilter` | 185.1201 | 199.1225 | 1.0756 | 0 |
| `BinaryMorphologicalOpeningImageFilter` | 147.3792 | 158.7814 | 1.0774 | 0 |
| `BinaryOpeningByReconstructionImageFilter` | 145.0643 | 163.7034 | 1.1285 | 0 |
| `BinaryPruningImageFilter` | 121.9754 | 125.4771 | 1.0287 | 0 |
| `DilateObjectMorphologyImageFilter` | 10.2133 | 10.6852 | 1.0462 | 0 |
| `FastIncrementalBinaryDilateImageFilter` | 66.3603 | 78.949 | 1.1897 | 0 |

### Filtering/ImageGradient（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DifferenceOfGaussiansGradientImageFilter` | 14.8242 | 24.0501 | 1.6224 | 0 |
| `GradientImageFilter` | 7.1421 | 16.8473 | 2.3589 | 0 |
| `GradientMagnitudeImageFilter` | 7.5732 | 16.699 | 2.205 | 1.88e-06 |
| `GradientRecursiveGaussianImageFilter` | 71.9297 | 1913.0263 | 26.5958 | 0 |
| `VectorGradientMagnitudeImageFilter` | 1426.3458 | 1473.6009 | 1.0331 | 0 |

### Filtering/LabelMap（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryImageToStatisticsLabelMapFilter` | 34.758 | 43.2962 | 1.2456 | 0 |
| `BinaryStatisticsKeepNObjectsImageFilter` | 49.5223 | 56.9899 | 1.1508 | 0 |
| `BinaryStatisticsOpeningImageFilter` | 49.0705 | 57.3682 | 1.1691 | 0 |
| `LabelMapMaskImageFilter` | 5.8719 | 12.6601 | 2.156 | 0 |
| `StatisticsRelabelImageFilter` | 149.0515 | 158.6134 | 1.0642 | 0 |

### Filtering/Smoothing（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinomialBlurImageFilter` | 99.729 | 108.2443 | 1.0854 | 0 |
| `BoxMeanImageFilter` | 8.5765 | 17.6443 | 2.0573 | 6.78e-06 |
| `MeanImageFilter` | 7.6497 | 17.659 | 2.3085 | 6.78e-06 |
| `MedianImageFilter` | 7.6595 | 16.8849 | 2.2044 | 0 |
| `RecursiveGaussianImageFilter` | 7.4214 | 17.4577 | 2.3524 | 7.63e-06 |

### Segmentation/RegionGrowing（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ConfidenceConnectedImageFilter` | 5.5238 | 14.3424 | 2.5965 | 0 |
| `ConnectedThresholdImageFilter` | 1.7891 | 10.111 | 5.6515 | 0 |
| `IsolatedConnectedImageFilter` | 10.1273 | 23.3932 | 2.3099 | 0 |
| `NeighborhoodConnectedImageFilter` | 4.0265 | 11.0069 | 2.7336 | 0 |
| `VectorConfidenceConnectedImageFilter` | 13.9792 | 25.1288 | 1.7976 | 0 |

### Filtering/Convolution（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ConvolutionImageFilter` | 8.6558 | 19.0096 | 2.1962 | 7.32e-06 |
| `FFTNormalizedCorrelationImageFilter` | 5151.8454 | 7419.7663 | 1.4402 | 3.28e-06 |
| `MaskedFFTNormalizedCorrelationImageFilter` | 5186.5041 | 7523.7422 | 1.4506 | 3.60e-07 |
| `NormalizedCorrelationImageFilter` | 7.402 | 16.8269 | 2.2733 | 0 |

### Registration/Common（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `KappaStatisticImageToImageMetric` | 155.6439 | 175.8355 | 1.1297 | 0 |
| `MatchCardinalityImageToImageMetric` | 6.9728 | 7.4052 | 1.062 | 0 |
| `MattesMutualInformationImageToImageMetric` | 59.183 | 76.5577 | 1.2936 | 0 |
| `MultiResolutionImageRegistrationMethod` | 160.0296 | 170.2191 | 1.0637 | 0 |

### Registration/RegistrationMethodsv4（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BSplineSyNImageRegistrationMethod` | 79.097 | 82.0444 | 1.0373 | 6.69e-07 |
| `ImageRegistrationMethodv4` | 42.5149 | 47.7497 | 1.1231 | 0 |
| `TimeVaryingBSplineVelocityFieldImageRegistrationMethod` | 56.5806 | 57.3002 | 1.0127 | 6.69e-07 |
| `TimeVaryingVelocityFieldImageRegistrationMethodv4` | 37.9707 | 38.7722 | 1.0211 | 0 |

### Segmentation/LabelVoting（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryMedianImageFilter` | 7.2032 | 7.7482 | 1.0757 | 0 |
| `VotingBinaryHoleFillingImageFilter` | 6.1869 | 6.8192 | 1.1022 | 0 |
| `VotingBinaryImageFilter` | 7.4482 | 7.5952 | 1.0197 | 0 |
| `VotingBinaryIterativeHoleFillingImageFilter` | 15.6605 | 17.2572 | 1.102 | 0 |

### Filtering/DisplacementField（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ComposeDisplacementFieldsImageFilter` | 8.0725 | 8.5497 | 1.0591 | 0 |
| `IterativeInverseDisplacementFieldImageFilter` | 1570.6699 | 1579.6286 | 1.0057 | 0 |
| `TransformToDisplacementFieldFilter` | 7.0308 | 7.2108 | 1.0256 | 0 |

### Filtering/DistanceMap（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ApproximateSignedDistanceMapImageFilter` | 151.8073 | 194.0331 | 1.2782 | 2.40e-07 |
| `FastChamferDistanceImageFilter` | 108.0727 | 120.1682 | 1.1119 | 0 |
| `IsoContourDistanceImageFilter` | 59.163 | 60.6433 | 1.025 | 1.00e-08 |

### Filtering/ImageCompare（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AbsoluteValueDifferenceImageFilter` | 6.9443 | 16.3843 | 2.3594 | 0 |
| `CheckerBoardImageFilter` | 6.8961 | 17.0266 | 2.469 | 0 |
| `SquaredDifferenceImageFilter` | 6.9845 | 16.2352 | 2.3245 | 0 |

### Filtering/ImageCompose（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ComposeImageFilter` | 7.316 | 14.1025 | 1.9276 | 0 |
| `JoinImageFilter` | 7.5759 | 15.4688 | 2.0418 | 0 |
| `JoinSeriesImageFilter` | 3.4578 | 11.715 | 3.388 | 0 |

### Filtering/ImageFilterBase（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `CastImageFilter` | 0.9164 | 0.9995 | 1.0907 | 0 |
| `NoiseImageFilter` | 7.1289 | 17.2271 | 2.4165 | 1.84e-06 |
| `VectorNeighborhoodOperatorImageFilter` | 7.9588 | 8.6944 | 1.0924 | 0 |

### Registration/Metricsv4（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `EuclideanDistancePointSetToPointSetMetricv4` | 29.9584 | 30.5074 | 1.0183 | 0 |
| `JensenHavrdaCharvatTsallisPointSetToPointSetMetricv4` | 25.5271 | 26.238 | 1.0279 | 0 |
| `MattesMutualInformationImageToImageMetricv4` | 115.6135 | 120.6213 | 1.0433 | 0 |

### Registration/PDEDeformable（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DemonsRegistrationFilter` | 140.9973 | 156.965 | 1.1132 | 0 |
| `DiffeomorphicDemonsRegistrationFilter` | 310.9294 | 361.338 | 1.1621 | 0 |
| `FastSymmetricForcesDemonsRegistrationFilter` | 176.9995 | 203.987 | 1.1525 | 0 |

### Segmentation/Watersheds（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `IsolatedWatershedImageFilter` | 3622.0152 | 3876.9398 | 1.0704 | 0 |
| `MorphologicalWatershedFromMarkersImageFilter` | 284.2614 | 314.2244 | 1.1054 | 0 |
| `TobogganImageFilter` | 163.2129 | 199.2148 | 1.2206 | 0 |

### Filtering/AnisotropicSmoothing（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `VectorCurvatureAnisotropicDiffusionImageFilter` | 304.9281 | 407.398 | 1.336 | 0 |
| `VectorGradientAnisotropicDiffusionImageFilter` | 311.9212 | 431.5919 | 1.3837 | 0 |

### Filtering/Path（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ContourExtractor2DImageFilter` | 103.7839 | 128.0944 | 1.2342 | 0 |
| `ExtractOrthogonalSwath2DImageFilter` | 0.3938 | 6.8024 | 17.2724 | 0 |

### Segmentation/Classifiers（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BayesianClassifierInitializationImageFilter` | 3601.6423 | 5040.4569 | 1.3995 | 0 |
| `ScalarImageKmeansImageFilter` | 3321.4448 | 3550.3349 | 1.0689 | 0 |

### Segmentation/ConnectedComponents（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `HardConnectedComponentImageFilter` | 28.7479 | 30.9096 | 1.0752 | 0 |
| `ThresholdMaximumConnectedComponentsImageFilter` | 942.2795 | 974.9261 | 1.0346 | 0 |

### Filtering/AntiAlias（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AntiAliasBinaryImageFilter` | 187.0664 | 209.3218 | 1.119 | 2.20e-07 |

### Filtering/Colormap（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ScalarToRGBColormapImageFilter` | 18.9314 | 26.331 | 1.3909 | 0 |

### Filtering/CurvatureFlow（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryMinMaxCurvatureFlowImageFilter` | 73.0241 | 80.9652 | 1.1087 | 0 |

### Filtering/ImageFrequency（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `FrequencyBandImageFilter` | 7.7491 | 8.7436 | 1.1284 | 0 |

### Filtering/QuadEdgeMeshFiltering（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DiscreteGaussianCurvatureQuadEdgeMeshFilter` | 21.8625 | 21.9784 | 1.0053 | 0 |

### Segmentation/DeformableMesh（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DeformableSimplexMesh3DFilter` | 46.6432 | 48.3299 | 1.0362 | 0 |

### Segmentation/Voronoi（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `VoronoiSegmentationRGBImageFilter` | 4.2095 | 4.3021 | 1.022 | 0 |

