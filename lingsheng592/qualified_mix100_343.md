# 灵昇 592 核：mixable100 双标合格函数（343）

来源：作业 **1960127**（cn05128，`mixable100_20261025_182701`），对已测混精可跑接口 **一轮**、每函数内层 **100** 次计时。

合格条件（同时）：

- 可判定的 `max_abs < 1e-5`
- `speedup = double_ms / float_ms > 1`

| 口径 | 数量 |
| --- | ---: |
| 候选可跑 | 468 |
| 本轮测量 OK | 462 |
| **双标合格（本文 / `qualified_mix100_343.tsv`）** | **343** |

送测跑法：`bash submit_mix100_343.sh`（只测这 343 个）。

## 按模块汇总

| 模块 | 个数 |
| --- | ---: |
| Filtering/ImageIntensity | 58 |
| Filtering/MathematicalMorphology | 28 |
| Filtering/ImageGrid | 27 |
| Filtering/LabelMap | 23 |
| Filtering/Thresholding | 19 |
| Filtering/ImageFeature | 16 |
| Segmentation/LevelSets | 16 |
| Filtering/ImageStatistics | 14 |
| Filtering/FFT | 12 |
| Registration/Common | 10 |
| Filtering/BinaryMathematicalMorphology | 8 |
| Filtering/DisplacementField | 7 |
| Filtering/QuadEdgeMeshFiltering | 7 |
| Filtering/DistanceMap | 6 |
| Filtering/Smoothing | 6 |
| Registration/Metricsv4 | 6 |
| Segmentation/LabelVoting | 6 |
| Filtering/ImageGradient | 5 |
| Registration/PDEDeformable | 5 |
| Segmentation/ConnectedComponents | 5 |
| Segmentation/RegionGrowing | 5 |
| Filtering/Convolution | 4 |
| Filtering/ImageCompare | 4 |
| Filtering/Path | 4 |
| Registration/RegistrationMethodsv4 | 4 |
| Filtering/ImageCompose | 3 |
| Filtering/ImageFilterBase | 3 |
| Segmentation/Classifiers | 3 |
| Segmentation/DeformableMesh | 3 |
| Segmentation/Voronoi | 3 |
| Segmentation/Watersheds | 3 |
| Filtering/Deconvolution | 2 |
| Filtering/DiffusionTensorImage | 2 |
| Filtering/ImageFusion | 2 |
| Filtering/ImageLabel | 2 |
| Filtering/ImageSources | 2 |
| Filtering/AnisotropicSmoothing | 1 |
| Filtering/AntiAlias | 1 |
| Filtering/BiasCorrection | 1 |
| Filtering/Colormap | 1 |
| Filtering/CurvatureFlow | 1 |
| Filtering/ImageFrequency | 1 |
| Filtering/SpatialFunction | 1 |
| Segmentation/KLMRegionGrowing | 1 |
| Segmentation/MarkovRandomFieldsClassifiers | 1 |
| Segmentation/SuperPixel | 1 |
| **合计** | **343** |

## 分模块名单

### Filtering/ImageIntensity（58）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AbsImageFilter` | 7.1774 | 18.4596 | 2.5719 | 0 |
| `AcosImageFilter` | 7.3811 | 18.9589 | 2.5686 | 4.00e-08 |
| `AddImageFilter` | 7.4482 | 19.1615 | 2.5726 | 0 |
| `AndImageFilter` | 7.472 | 7.7078 | 1.0316 | 0 |
| `AsinImageFilter` | 7.6149 | 19.5584 | 2.5684 | 4.00e-08 |
| `Atan2ImageFilter` | 6.9865 | 17.748 | 2.5403 | 2.00e-08 |
| `AtanImageFilter` | 7.495 | 19.1496 | 2.555 | 6.00e-08 |
| `BinaryMagnitudeImageFilter` | 7.3943 | 7.881 | 1.0658 | 0 |
| `BoundedReciprocalImageFilter` | 6.9928 | 17.9751 | 2.5705 | 1.00e-08 |
| `ClampImageFilter` | 6.8918 | 17.5385 | 2.5448 | 0 |
| `ComplexToImaginaryImageFilter` | 7.4862 | 7.9144 | 1.0572 | 0 |
| `ComplexToModulusImageFilter` | 7.3882 | 7.8125 | 1.0574 | 0 |
| `ComplexToPhaseImageFilter` | 7.5763 | 7.9708 | 1.0521 | 0 |
| `ComplexToRealImageFilter` | 7.3059 | 7.859 | 1.0757 | 0 |
| `ConstrainedValueAdditionImageFilter` | 6.9172 | 17.5141 | 2.532 | 0 |
| `ConstrainedValueDifferenceImageFilter` | 7.2191 | 15.6545 | 2.1685 | 0 |
| `CosImageFilter` | 7.3784 | 18.8202 | 2.5507 | 3.00e-08 |
| `DivideOrZeroOutImageFilter` | 6.755 | 17.7861 | 2.633 | 0 |
| `EdgePotentialImageFilter` | 16.1353 | 16.9099 | 1.048 | 0 |
| `ExpNegativeImageFilter` | 6.8865 | 17.8184 | 2.5874 | 1.00e-08 |
| `HistogramMatchingImageFilter` | 186.5399 | 221.2243 | 1.1859 | 0 |
| `IntensityWindowingImageFilter` | 6.7525 | 17.6707 | 2.6169 | 0 |
| `Log10ImageFilter` | 6.9496 | 17.9363 | 2.5809 | 1.20e-07 |
| `LogImageFilter` | 7.4377 | 19.3365 | 2.5998 | 2.40e-07 |
| `MagnitudeAndPhaseToComplexImageFilter` | 8.3116 | 9.4865 | 1.1414 | 0 |
| `MaskImageFilter` | 7.8206 | 19.242 | 2.4604 | 0 |
| `MaskNegatedImageFilter` | 6.9785 | 17.5584 | 2.5161 | 0 |
| `MatrixIndexSelectionImageFilter` | 0.8474 | 0.8598 | 1.0146 | 0 |
| `MaximumImageFilter` | 6.8496 | 17.7717 | 2.5946 | 0 |
| `MinimumImageFilter` | 6.8188 | 17.6457 | 2.5878 | 0 |
| `ModulusImageFilter` | 6.8458 | 6.9596 | 1.0166 | 0 |
| `MultiplyImageFilter` | 6.7665 | 17.6812 | 2.613 | 0 |
| `NaryAddImageFilter` | 7.2944 | 15.5017 | 2.1252 | 0 |
| `NaryMaximumImageFilter` | 7.2424 | 15.4669 | 2.1356 | 0 |
| `NormalizeImageFilter` | 14.6446 | 26.5749 | 1.8147 | 6.00e-08 |
| `NormalizeToConstantImageFilter` | 14.2264 | 25.4071 | 1.7859 | 0 |
| `NotImageFilter` | 7.4803 | 7.6075 | 1.017 | 0 |
| `OrImageFilter` | 7.4927 | 7.6882 | 1.0261 | 0 |
| `PolylineMask2DImageFilter` | 0.0866 | 0.089 | 1.0287 | 0 |
| `PolylineMaskImageFilter` | 0.5066 | 0.5074 | 1.0014 | 0 |
| `RescaleIntensityImageFilter` | 13.9306 | 34.9783 | 2.5109 | 3.00e-08 |
| `RGBToLuminanceImageFilter` | 24.4749 | 25.1262 | 1.0266 | 0 |
| `RoundImageFilter` | 6.7249 | 17.5534 | 2.6102 | 0 |
| `ShiftScaleImageFilter` | 6.8788 | 17.9074 | 2.6033 | 0 |
| `SinImageFilter` | 7.3639 | 18.7695 | 2.5489 | 3.00e-08 |
| `SqrtImageFilter` | 7.3714 | 19.0296 | 2.5815 | 4.70e-07 |
| `SquareImageFilter` | 6.6463 | 17.4757 | 2.6294 | 0 |
| `SubtractImageFilter` | 6.7626 | 17.7461 | 2.6242 | 0 |
| `SymmetricEigenAnalysisImageFilter` | 0.8329 | 0.8458 | 1.0156 | 0 |
| `TanImageFilter` | 7.6833 | 19.2091 | 2.5001 | 4.99e-06 |
| `TernaryAddImageFilter` | 7.3135 | 16.1467 | 2.2078 | 0 |
| `TernaryMagnitudeImageFilter` | 7.4306 | 7.8311 | 1.0539 | 0 |
| `TernaryMagnitudeSquaredImageFilter` | 7.3815 | 7.7794 | 1.0539 | 0 |
| `VectorExpandImageFilter` | 0.8343 | 0.8519 | 1.0211 | 0 |
| `VectorIndexSelectionCastImageFilter` | 7.4996 | 8.0679 | 1.0758 | 0 |
| `VectorMagnitudeImageFilter` | 14.6754 | 23.6849 | 1.6139 | 0 |
| `WeightedAddImageFilter` | 6.9552 | 17.4327 | 2.5064 | 0 |
| `XorImageFilter` | 7.3397 | 7.5068 | 1.0228 | 0 |

### Filtering/MathematicalMorphology（28）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BasicDilateImageFilter` | 7.5158 | 7.9948 | 1.0637 | 0 |
| `BasicErodeImageFilter` | 7.9101 | 8.3343 | 1.0536 | 0 |
| `BlackTopHatImageFilter` | 40.4668 | 57.7693 | 1.4276 | 0 |
| `GrayscaleConnectedClosingImageFilter` | 597.8313 | 681.6718 | 1.1402 | 0 |
| `GrayscaleConnectedOpeningImageFilter` | 919.3983 | 1073.096 | 1.1672 | 0 |
| `GrayscaleDilateImageFilter` | 7.8685 | 18.9462 | 2.4078 | 0 |
| `GrayscaleErodeImageFilter` | 7.7373 | 18.8689 | 2.4387 | 0 |
| `GrayscaleFillholeImageFilter` | 483.1224 | 560.7338 | 1.1606 | 0 |
| `GrayscaleFunctionDilateImageFilter` | 8.1886 | 8.3693 | 1.0221 | 0 |
| `GrayscaleFunctionErodeImageFilter` | 7.5419 | 7.9044 | 1.0481 | 0 |
| `GrayscaleGrindPeakImageFilter` | 274.7577 | 299.5604 | 1.0903 | 0 |
| `GrayscaleMorphologicalClosingImageFilter` | 34.978 | 51.2621 | 1.4656 | 0 |
| `GrayscaleMorphologicalOpeningImageFilter` | 34.8109 | 50.3124 | 1.4453 | 0 |
| `HConcaveImageFilter` | 243.7091 | 245.2484 | 1.0063 | 0 |
| `MaskedMovingHistogramImageFilter` | 0.8704 | 0.8828 | 1.0143 | 0 |
| `MaskedRankImageFilter` | 7.6894 | 8.318 | 1.0818 | 0 |
| `MorphologicalGradientImageFilter` | 24.7932 | 39.2738 | 1.5841 | 0 |
| `MovingHistogramDilateImageFilter` | 10.0648 | 10.3765 | 1.031 | 0 |
| `MovingHistogramErodeImageFilter` | 9.0898 | 9.8304 | 1.0815 | 0 |
| `RankImageFilter` | 8.8292 | 20.2672 | 2.2955 | 0 |
| `ReconstructionByDilationImageFilter` | 173.1255 | 194.8463 | 1.1255 | 0 |
| `RegionalMaximaImageFilter` | 128.6556 | 149.0013 | 1.1581 | 0 |
| `RegionalMinimaImageFilter` | 130.1619 | 151.1026 | 1.1609 | 0 |
| `ValuedRegionalMaximaImageFilter` | 130.0481 | 134.4776 | 1.0341 | 0 |
| `ValuedRegionalMinimaImageFilter` | 128.8475 | 137.678 | 1.0685 | 0 |
| `VanHerkGilWermanDilateImageFilter` | 7.5227 | 7.9028 | 1.0505 | 0 |
| `VanHerkGilWermanErodeImageFilter` | 7.6647 | 8.1004 | 1.0568 | 0 |
| `WhiteTopHatImageFilter` | 42.9482 | 56.0317 | 1.3046 | 0 |

### Filtering/ImageGrid（27）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinShrinkImageFilter` | 6.9125 | 15.2724 | 2.2094 | 0 |
| `BSplineDownsampleImageFilter` | 13.6884 | 27.2687 | 1.9921 | 0 |
| `BSplineScatteredDataPointSetToImageFilter` | 30.0591 | 31.3139 | 1.0417 | 0 |
| `BSplineUpsampleImageFilter` | 49.9529 | 85.6511 | 1.7146 | 0 |
| `ChangeInformationImageFilter` | 0.0271 | 6.7403 | 248.3356 | 0 |
| `ConstantPadImageFilter` | 6.716 | 15.1087 | 2.2497 | 0 |
| `CropImageFilter` | 7.0346 | 18.3869 | 2.6138 | 0 |
| `CyclicShiftImageFilter` | 6.8622 | 17.7825 | 2.5914 | 0 |
| `ExpandImageFilter` | 7.0582 | 17.6058 | 2.4944 | 0 |
| `FlipImageFilter` | 7.4452 | 19.2949 | 2.5916 | 0 |
| `InterpolateImageFilter` | 37.867 | 46.4674 | 1.2271 | 0 |
| `InterpolateImagePointsFilter` | 1.2098 | 1.2277 | 1.0148 | 0 |
| `MirrorPadImageFilter` | 6.9097 | 15.1792 | 2.1968 | 0 |
| `OrientImageFilter` | 0.3597 | 0.3618 | 1.0057 | 0 |
| `PadImageFilter` | 6.8359 | 7.1735 | 1.0494 | 0 |
| `PasteImageFilter` | 7.0541 | 17.9417 | 2.5434 | 0 |
| `PermuteAxesImageFilter` | 7.2262 | 18.9598 | 2.6237 | 0 |
| `RegionOfInterestImageFilter` | 6.3758 | 16.5935 | 2.6026 | 0 |
| `ResampleImageFilter` | 7.3763 | 18.8884 | 2.5607 | 0 |
| `ShrinkImageFilter` | 6.7775 | 15.0838 | 2.2256 | 0 |
| `SliceBySliceImageFilter` | 3.4737 | 3.6264 | 1.044 | 0 |
| `SliceImageFilter` | 7.3811 | 7.7217 | 1.0462 | 0 |
| `TileImageFilter` | 18.254 | 28.4497 | 1.5585 | 0 |
| `WarpImageFilter` | 7.4777 | 18.9723 | 2.5372 | 7.93e-06 |
| `WarpVectorImageFilter` | 7.51 | 7.9838 | 1.0631 | 0 |
| `WrapPadImageFilter` | 6.8497 | 15.3196 | 2.2365 | 0 |
| `ZeroFluxNeumannPadImageFilter` | 6.7502 | 15.1349 | 2.2422 | 0 |

### Filtering/LabelMap（23）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryFillholeImageFilter` | 64.1528 | 64.7832 | 1.0098 | 0 |
| `BinaryGrindPeakImageFilter` | 63.989 | 64.2668 | 1.0043 | 0 |
| `BinaryImageToStatisticsLabelMapFilter` | 35.6702 | 45.2324 | 1.2681 | 0 |
| `BinaryNotImageFilter` | 6.8519 | 7.0074 | 1.0227 | 0 |
| `BinaryShapeKeepNObjectsImageFilter` | 79.0661 | 79.9936 | 1.0117 | 0 |
| `BinaryShapeOpeningImageFilter` | 43.0835 | 43.3134 | 1.0053 | 0 |
| `BinaryStatisticsKeepNObjectsImageFilter` | 51.6716 | 59.8493 | 1.1583 | 0 |
| `BinaryStatisticsOpeningImageFilter` | 50.2073 | 54.6331 | 1.0882 | 0 |
| `ChangeLabelLabelMapFilter` | 58.4156 | 58.8812 | 1.008 | 0 |
| `LabelImageToLabelMapFilter` | 13.2202 | 13.4448 | 1.017 | 0 |
| `LabelImageToShapeLabelMapFilter` | 18.725 | 19.0371 | 1.0167 | 0 |
| `LabelImageToStatisticsLabelMapFilter` | 169.9398 | 178.676 | 1.0514 | 0 |
| `LabelMapMaskImageFilter` | 6.1929 | 12.8899 | 2.0814 | 0 |
| `LabelMapToAttributeImageFilter` | 0.0263 | 0.0264 | 1.003 | 0 |
| `LabelMapToBinaryImageFilter` | 61.0587 | 62.3069 | 1.0204 | 0 |
| `LabelMapToLabelImageFilter` | 65.8551 | 66.6449 | 1.012 | 0 |
| `LabelShapeOpeningImageFilter` | 34.646 | 35.6096 | 1.0278 | 0 |
| `LabelStatisticsKeepNObjectsImageFilter` | 183.8923 | 210.077 | 1.1424 | 0 |
| `LabelStatisticsOpeningImageFilter` | 195.6148 | 220.9246 | 1.1294 | 0 |
| `RelabelLabelMapFilter` | 52.3774 | 53.0277 | 1.0124 | 0 |
| `ShapeKeepNObjectsLabelMapFilter` | 58.245 | 58.6564 | 1.0071 | 0 |
| `ShapeRelabelImageFilter` | 71.0206 | 71.3989 | 1.0053 | 0 |
| `StatisticsRelabelImageFilter` | 191.719 | 266.8002 | 1.3916 | 0 |

### Filtering/Thresholding（19）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryThresholdImageFilter` | 6.7842 | 17.7914 | 2.6225 | 0 |
| `BinaryThresholdProjectionImageFilter` | 7.1911 | 16.9844 | 2.3619 | 0 |
| `HistogramThresholdImageFilter` | 23.495 | 34.8119 | 1.4817 | 0 |
| `HuangThresholdImageFilter` | 25.5601 | 37.3978 | 1.4631 | 0 |
| `IntermodesThresholdImageFilter` | 24.4845 | 36.3639 | 1.4852 | 0 |
| `IsoDataThresholdImageFilter` | 25.029 | 37.7203 | 1.5071 | 0 |
| `KappaSigmaThresholdImageFilter` | 34.986 | 90.6627 | 2.5914 | 0 |
| `KittlerIllingworthThresholdImageFilter` | 24.36 | 36.5394 | 1.5 | 0 |
| `LiThresholdImageFilter` | 24.0173 | 35.9907 | 1.4985 | 0 |
| `MaximumEntropyThresholdImageFilter` | 27.2887 | 38.7463 | 1.4199 | 0 |
| `MomentsThresholdImageFilter` | 24.2476 | 36.4112 | 1.5016 | 0 |
| `OtsuMultipleThresholdsImageFilter` | 112.0332 | 145.7917 | 1.3013 | 0 |
| `OtsuThresholdImageFilter` | 23.897 | 35.8842 | 1.5016 | 0 |
| `RenyiEntropyThresholdImageFilter` | 28.2145 | 40.1579 | 1.4233 | 0 |
| `ShanbhagThresholdImageFilter` | 25.4154 | 37.5728 | 1.4783 | 0 |
| `ThresholdImageFilter` | 7.0474 | 17.7233 | 2.5149 | 0 |
| `ThresholdLabelerImageFilter` | 7.0462 | 17.6443 | 2.5041 | 0 |
| `TriangleThresholdImageFilter` | 24.3114 | 36.4891 | 1.5009 | 0 |
| `YenThresholdImageFilter` | 24.4712 | 36.5399 | 1.4932 | 0 |

### Filtering/ImageFeature（16）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `CannyEdgeDetectionImageFilter` | 65.1222 | 84.4285 | 1.2965 | 0 |
| `DerivativeImageFilter` | 7.3515 | 18.1526 | 2.4692 | 0 |
| `DiscreteGaussianDerivativeImageFilter` | 35.2768 | 49.4916 | 1.403 | 4.50e-07 |
| `GradientVectorFlowImageFilter` | 7.5361 | 8.0357 | 1.0663 | 0 |
| `Hessian3DToVesselnessMeasureImageFilter` | 7.3833 | 7.5113 | 1.0173 | 0 |
| `HessianRecursiveGaussianImageFilter` | 107.6577 | 125.953 | 1.1699 | 0 |
| `HessianToObjectnessMeasureImageFilter` | 124.6901 | 125.7752 | 1.0087 | 0 |
| `HoughTransform2DLinesImageFilter` | 2.1919 | 2.2653 | 1.0335 | 0 |
| `LaplacianImageFilter` | 7.517 | 18.7777 | 2.498 | 0 |
| `LaplacianRecursiveGaussianImageFilter` | 58.7384 | 74.7553 | 1.2727 | 0 |
| `LaplacianSharpeningImageFilter` | 45.5328 | 57.9283 | 1.2722 | 7.23e-06 |
| `MaskFeaturePointSelectionFilter` | 12.9574 | 12.9599 | 1.0002 | 0 |
| `MultiScaleHessianBasedMeasureImageFilter` | 16.4154 | 16.9495 | 1.0325 | 0 |
| `SimpleContourExtractorImageFilter` | 6.8409 | 17.191 | 2.513 | 0 |
| `ZeroCrossingBasedEdgeDetectionImageFilter` | 37.2978 | 52.2347 | 1.4005 | 0 |
| `ZeroCrossingImageFilter` | 7.2773 | 18.7558 | 2.5773 | 0 |

### Segmentation/LevelSets（16）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryMaskToNarrowBandPointSetFilter` | 116.7202 | 120.1717 | 1.0296 | 0 |
| `CannySegmentationLevelSetImageFilter` | 939.5997 | 958.4137 | 1.02 | 0 |
| `CollidingFrontsImageFilter` | 2938.6107 | 2944.579 | 1.002 | 0 |
| `CurvesLevelSetImageFilter` | 210.6973 | 213.2515 | 1.0121 | 0 |
| `ExtensionVelocitiesImageFilter` | 1047.8594 | 1138.5888 | 1.0866 | 0 |
| `GeodesicActiveContourLevelSetImageFilter` | 210.0402 | 212.0632 | 1.0096 | 0 |
| `GeodesicActiveContourShapePriorLevelSetImageFilter` | 13.2839 | 14.0238 | 1.0557 | 0 |
| `ImplicitManifoldNormalVectorFilter` | 88.4157 | 89.3367 | 1.0104 | 0 |
| `IsotropicFourthOrderLevelSetImageFilter` | 7644.2549 | 7733.0658 | 1.0116 | 0 |
| `LaplacianSegmentationLevelSetImageFilter` | 81.4476 | 90.4292 | 1.1103 | 0 |
| `NarrowBandCurvesLevelSetImageFilter` | 410.669 | 412.6427 | 1.0048 | 0 |
| `NarrowBandThresholdSegmentationLevelSetImageFilter` | 267.4026 | 280.8392 | 1.0502 | 0 |
| `ShapeDetectionLevelSetImageFilter` | 70.6837 | 72.2308 | 1.0219 | 0 |
| `ThresholdSegmentationLevelSetImageFilter` | 71.8833 | 88.3352 | 1.2289 | 0 |
| `UnsharpMaskLevelSetImageFilter` | 829.6954 | 834.6233 | 1.0059 | 0 |
| `VectorThresholdSegmentationLevelSetImageFilter` | 2.3883 | 2.5393 | 1.0632 | 0 |

### Filtering/ImageStatistics（14）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AccumulateImageFilter` | 19.9528 | 33.0475 | 1.6563 | 0 |
| `AdaptiveHistogramEqualizationImageFilter` | 16.5074 | 28.4335 | 1.7225 | 7.63e-06 |
| `BinaryProjectionImageFilter` | 7.245 | 7.6285 | 1.0529 | 0 |
| `GetAverageSliceImageFilter` | 0.2809 | 0.2899 | 1.0321 | 0 |
| `LabelOverlapMeasuresImageFilter` | 7.4532 | 7.5904 | 1.0184 | 0 |
| `LabelStatisticsImageFilter` | 7.6218 | 7.8086 | 1.0245 | 0 |
| `MaximumProjectionImageFilter` | 6.9673 | 15.9315 | 2.2866 | 0 |
| `MeanProjectionImageFilter` | 7.0735 | 16.031 | 2.2664 | 0 |
| `MedianProjectionImageFilter` | 7.3593 | 16.8548 | 2.2903 | 0 |
| `MinimumMaximumImageFilter` | 7.2201 | 15.6986 | 2.1743 | 0 |
| `MinimumProjectionImageFilter` | 7.0781 | 16.1108 | 2.2761 | 0 |
| `StandardDeviationProjectionImageFilter` | 7.247 | 16.976 | 2.3425 | 0 |
| `StatisticsImageFilter` | 7.2742 | 15.583 | 2.1422 | 0 |
| `SumProjectionImageFilter` | 7.1072 | 15.8658 | 2.2324 | 0 |

### Filtering/FFT（12）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `FFTPadImageFilter` | 6.7454 | 17.7495 | 2.6314 | 0 |
| `FFTShiftImageFilter` | 6.8705 | 17.8895 | 2.6038 | 0 |
| `FullToHalfHermitianImageFilter` | 7.9726 | 8.5901 | 1.0775 | 0 |
| `HalfToFullHermitianImageFilter` | 8.3571 | 9.3377 | 1.1173 | 0 |
| `VnlComplexToComplex1DFFTImageFilter` | 0.8205 | 0.8355 | 1.0183 | 0 |
| `VnlComplexToComplexFFTImageFilter` | 0.9771 | 1.0235 | 1.0475 | 0 |
| `VnlForward1DFFTImageFilter` | 8.7849 | 9.8293 | 1.1189 | 0 |
| `VnlForwardFFTImageFilter` | 88.1026 | 109.4435 | 1.2422 | 0 |
| `VnlHalfHermitianToRealInverseFFTImageFilter` | 93.6949 | 105.7885 | 1.1291 | 0 |
| `VnlInverse1DFFTImageFilter` | 7.6922 | 8.1986 | 1.0658 | 0 |
| `VnlInverseFFTImageFilter` | 162.5306 | 197.7358 | 1.2166 | 0 |
| `VnlRealToHalfHermitianForwardFFTImageFilter` | 76.6138 | 85.7951 | 1.1198 | 0 |

### Registration/Common（10）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BlockMatchingImageFilter` | 8.0586 | 8.1821 | 1.0153 | 0 |
| `EuclideanDistancePointMetric` | 2665.8444 | 2666.7153 | 1.0003 | 0 |
| `GradientDifferenceImageToImageMetric` | 295.9951 | 296.9357 | 1.0032 | 0 |
| `ImageToSpatialObjectRegistrationMethod` | 2.8982 | 2.9839 | 1.0296 | 0 |
| `KappaStatisticImageToImageMetric` | 151.6276 | 184.5084 | 1.2169 | 0 |
| `MatchCardinalityImageToImageMetric` | 7.9518 | 8.509 | 1.0701 | 0 |
| `MattesMutualInformationImageToImageMetric` | 59.697 | 71.0427 | 1.1901 | 0 |
| `MultiResolutionImageRegistrationMethod` | 186.4104 | 189.9933 | 1.0192 | 0 |
| `MutualInformationImageToImageMetric` | 19260.6065 | 19270.4386 | 1.0005 | 0 |
| `PointSetToPointSetRegistrationMethod` | 101.5023 | 101.8144 | 1.0031 | 0 |

### Filtering/BinaryMathematicalMorphology（8）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryClosingByReconstructionImageFilter` | 150.3758 | 177.037 | 1.1773 | 0 |
| `BinaryDilateImageFilter` | 67.025 | 78.1874 | 1.1665 | 0 |
| `BinaryMorphologicalClosingImageFilter` | 184.3617 | 213.2274 | 1.1566 | 0 |
| `BinaryMorphologicalOpeningImageFilter` | 146.7656 | 171.0224 | 1.1653 | 0 |
| `BinaryOpeningByReconstructionImageFilter` | 145.3069 | 157.069 | 1.0809 | 0 |
| `BinaryPruningImageFilter` | 123.0303 | 124.5654 | 1.0125 | 0 |
| `DilateObjectMorphologyImageFilter` | 8.3253 | 8.9701 | 1.0774 | 0 |
| `FastIncrementalBinaryDilateImageFilter` | 69.3818 | 86.2061 | 1.2425 | 0 |

### Filtering/DisplacementField（7）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ComposeDisplacementFieldsImageFilter` | 8.6653 | 9.8074 | 1.1318 | 0 |
| `DisplacementFieldJacobianDeterminantFilter` | 16.973 | 19.1184 | 1.1264 | 0 |
| `DisplacementFieldToBSplineImageFilter` | 28.4487 | 28.7961 | 1.0122 | 0 |
| `InvertDisplacementFieldImageFilter` | 96.6974 | 102.9845 | 1.065 | 0 |
| `IterativeInverseDisplacementFieldImageFilter` | 1587.3883 | 1607.367 | 1.0126 | 0 |
| `TimeVaryingVelocityFieldIntegrationImageFilter` | 0.481 | 0.4914 | 1.0216 | 0 |
| `TransformToDisplacementFieldFilter` | 7.2367 | 7.6139 | 1.0521 | 0 |

### Filtering/QuadEdgeMeshFiltering（7）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `CleanQuadEdgeMeshFilter` | 55.8647 | 56.2431 | 1.0068 | 0 |
| `DelaunayConformingQuadEdgeMeshFilter` | 17.0262 | 17.129 | 1.006 | 0 |
| `DiscreteMaximumCurvatureQuadEdgeMeshFilter` | 27.1224 | 27.3203 | 1.0073 | 0 |
| `ParameterizationQuadEdgeMeshFilter` | 3.9355 | 3.9796 | 1.0112 | 0 |
| `QuadricDecimationQuadEdgeMeshFilter` | 97.6891 | 97.8216 | 1.0014 | 0 |
| `SmoothingQuadEdgeMeshFilter` | 21.7195 | 21.9665 | 1.0114 | 0 |
| `SquaredEdgeLengthDecimationQuadEdgeMeshFilter` | 12.9363 | 13.1397 | 1.0157 | 0 |

### Filtering/DistanceMap（6）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ApproximateSignedDistanceMapImageFilter` | 154.8997 | 172.3599 | 1.1127 | 2.40e-07 |
| `ContourDirectedMeanDistanceImageFilter` | 65.512 | 65.9122 | 1.0061 | 0 |
| `ContourMeanDistanceImageFilter` | 137.2527 | 137.9584 | 1.0051 | 0 |
| `FastChamferDistanceImageFilter` | 106.3343 | 122.4151 | 1.1512 | 0 |
| `HausdorffDistanceImageFilter` | 129.1065 | 130.7289 | 1.0126 | 0 |
| `IsoContourDistanceImageFilter` | 65.5124 | 66.4866 | 1.0149 | 1.00e-08 |

### Filtering/Smoothing（6）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinomialBlurImageFilter` | 96.5992 | 108.627 | 1.1245 | 0 |
| `BoxMeanImageFilter` | 9.0453 | 20.9245 | 2.3133 | 6.78e-06 |
| `MeanImageFilter` | 7.4849 | 19.3602 | 2.5866 | 6.78e-06 |
| `MedianImageFilter` | 7.6392 | 18.763 | 2.4561 | 0 |
| `RecursiveGaussianImageFilter` | 7.3304 | 18.0891 | 2.4677 | 7.63e-06 |
| `SmoothingRecursiveGaussianImageFilter` | 15.2465 | 1654.1827 | 108.4961 | 0 |

### Registration/Metricsv4（6）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DemonsImageToImageMetricv4` | 89.2499 | 89.9367 | 1.0077 | 0 |
| `EuclideanDistancePointSetToPointSetMetricv4` | 31.7748 | 32.3879 | 1.0193 | 0 |
| `ExpectationBasedPointSetToPointSetMetricv4` | 8.4539 | 8.8435 | 1.0461 | 0 |
| `JensenHavrdaCharvatTsallisPointSetToPointSetMetricv4` | 28.6948 | 29.1848 | 1.0171 | 0 |
| `MattesMutualInformationImageToImageMetricv4` | 110.8558 | 121.1611 | 1.093 | 0 |
| `ObjectToObjectMultiMetricv4` | 3446.9599 | 3496.3452 | 1.0143 | 0 |

### Segmentation/LabelVoting（6）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryMedianImageFilter` | 7.2968 | 7.8867 | 1.0808 | 0 |
| `LabelVotingImageFilter` | 48.35 | 49.2509 | 1.0186 | 0 |
| `MultiLabelSTAPLEImageFilter` | 172.5343 | 172.6714 | 1.0008 | 0 |
| `VotingBinaryHoleFillingImageFilter` | 6.5125 | 6.9577 | 1.0684 | 0 |
| `VotingBinaryImageFilter` | 7.4331 | 7.9935 | 1.0754 | 0 |
| `VotingBinaryIterativeHoleFillingImageFilter` | 15.2806 | 18.6481 | 1.2204 | 0 |

### Filtering/ImageGradient（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DifferenceOfGaussiansGradientImageFilter` | 14.7884 | 23.7265 | 1.6044 | 0 |
| `GradientImageFilter` | 7.368 | 18.7582 | 2.5459 | 0 |
| `GradientMagnitudeImageFilter` | 7.4481 | 18.5621 | 2.4922 | 1.88e-06 |
| `GradientMagnitudeRecursiveGaussianImageFilter` | 101.2948 | 1687.0712 | 16.6551 | 1.87e-06 |
| `GradientRecursiveGaussianImageFilter` | 73.9909 | 1717.9005 | 23.2177 | 0 |

### Registration/PDEDeformable（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DemonsRegistrationFilter` | 142.1909 | 155.4551 | 1.0933 | 0 |
| `DiffeomorphicDemonsRegistrationFilter` | 325.2405 | 362.2688 | 1.1138 | 0 |
| `FastSymmetricForcesDemonsRegistrationFilter` | 207.3043 | 237.6735 | 1.1465 | 0 |
| `LevelSetMotionRegistrationFilter` | 126.0245 | 140.0774 | 1.1115 | 0 |
| `SymmetricForcesDemonsRegistrationFilter` | 186.7194 | 230.7268 | 1.2357 | 0 |

### Segmentation/ConnectedComponents（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ConnectedComponentImageFilter` | 34.4505 | 35.0219 | 1.0166 | 0 |
| `HardConnectedComponentImageFilter` | 31.0313 | 37.2619 | 1.2008 | 0 |
| `RelabelComponentImageFilter` | 14.4503 | 14.8987 | 1.031 | 0 |
| `ScalarConnectedComponentImageFilter` | 2085.9965 | 2119.2254 | 1.0159 | 0 |
| `ThresholdMaximumConnectedComponentsImageFilter` | 957.9947 | 1060.8877 | 1.1074 | 0 |

### Segmentation/RegionGrowing（5）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ConfidenceConnectedImageFilter` | 2.8159 | 11.2848 | 4.0076 | 0 |
| `ConnectedThresholdImageFilter` | 0.3758 | 7.2778 | 19.3687 | 0 |
| `IsolatedConnectedImageFilter` | 9.0172 | 21.5988 | 2.3953 | 0 |
| `NeighborhoodConnectedImageFilter` | 1.0801 | 8.1017 | 7.5005 | 0 |
| `VectorConfidenceConnectedImageFilter` | 9.6169 | 20.9601 | 2.1795 | 0 |

### Filtering/Convolution（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ConvolutionImageFilter` | 8.9862 | 22.3453 | 2.4866 | 7.32e-06 |
| `FFTNormalizedCorrelationImageFilter` | 5325.3491 | 6376.2912 | 1.1973 | 3.28e-06 |
| `MaskedFFTNormalizedCorrelationImageFilter` | 5353.3099 | 6139.8889 | 1.1469 | 3.60e-07 |
| `NormalizedCorrelationImageFilter` | 7.4043 | 17.94 | 2.4229 | 0 |

### Filtering/ImageCompare（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AbsoluteValueDifferenceImageFilter` | 6.8256 | 17.8062 | 2.6087 | 0 |
| `CheckerBoardImageFilter` | 6.9535 | 17.4266 | 2.5062 | 0 |
| `SimilarityIndexImageFilter` | 15.4777 | 15.7538 | 1.0178 | 0 |
| `SquaredDifferenceImageFilter` | 6.8122 | 17.6744 | 2.5945 | 0 |

### Filtering/Path（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ContourExtractor2DImageFilter` | 103.2461 | 149.4596 | 1.4476 | 0 |
| `ExtractOrthogonalSwath2DImageFilter` | 0.3319 | 7.4326 | 22.3938 | 0 |
| `OrthogonalSwath2DPathFilter` | 27.5161 | 28.0102 | 1.018 | 0 |
| `PathToImageFilter` | 0.7164 | 0.7204 | 1.0055 | 0 |

### Registration/RegistrationMethodsv4（4）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BSplineSyNImageRegistrationMethod` | 91.4487 | 93.8253 | 1.026 | 6.69e-07 |
| `ImageRegistrationMethodv4` | 52.176 | 57.357 | 1.0993 | 0 |
| `SyNImageRegistrationMethod` | 55.9297 | 56.3525 | 1.0076 | 0 |
| `TimeVaryingVelocityFieldImageRegistrationMethodv4` | 42.2217 | 43.4608 | 1.0293 | 0 |

### Filtering/ImageCompose（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ComposeImageFilter` | 7.157 | 15.0314 | 2.1002 | 0 |
| `JoinImageFilter` | 7.4125 | 16.0881 | 2.1704 | 0 |
| `JoinSeriesImageFilter` | 1.734 | 10.5947 | 6.1101 | 0 |

### Filtering/ImageFilterBase（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `CastImageFilter` | 0.8372 | 0.8536 | 1.0195 | 0 |
| `NoiseImageFilter` | 7.3147 | 18.3597 | 2.51 | 1.84e-06 |
| `VectorNeighborhoodOperatorImageFilter` | 8.9884 | 10.0416 | 1.1172 | 0 |

### Segmentation/Classifiers（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BayesianClassifierImageFilter` | 9.0715 | 9.2812 | 1.0231 | 0 |
| `BayesianClassifierInitializationImageFilter` | 3594.0144 | 4743.0595 | 1.3197 | 0 |
| `ScalarImageKmeansImageFilter` | 3299.0987 | 3737.4441 | 1.1329 | 0 |

### Segmentation/DeformableMesh（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DeformableSimplexMesh3DBalloonForceFilter` | 15.4477 | 16.8998 | 1.094 | 0 |
| `DeformableSimplexMesh3DFilter` | 54.1901 | 56.7645 | 1.0475 | 0 |
| `DeformableSimplexMesh3DGradientConstraintForceFilter` | 16.304 | 17.9515 | 1.1011 | 0 |

### Segmentation/Voronoi（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `VoronoiPartitioningImageFilter` | 2.3185 | 2.3735 | 1.0237 | 0 |
| `VoronoiSegmentationImageFilter` | 1.9994 | 2.0284 | 1.0145 | 0 |
| `VoronoiSegmentationRGBImageFilter` | 4.404 | 4.4806 | 1.0174 | 0 |

### Segmentation/Watersheds（3）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `IsolatedWatershedImageFilter` | 3555.1 | 3715.6886 | 1.0452 | 0 |
| `MorphologicalWatershedFromMarkersImageFilter` | 278.9859 | 336.5047 | 1.2062 | 0 |
| `TobogganImageFilter` | 160.57 | 194.9052 | 1.2138 | 0 |

### Filtering/Deconvolution（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `InverseDeconvolutionImageFilter` | 357.9965 | 374.4447 | 1.0459 | 0 |
| `RichardsonLucyDeconvolutionImageFilter` | 1418.9513 | 1493.4744 | 1.0525 | 0 |

### Filtering/DiffusionTensorImage（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `DiffusionTensor3DReconstructionImageFilter` | 113.9139 | 114.1611 | 1.0022 | 0 |
| `TensorFractionalAnisotropyImageFilter` | 0.1536 | 0.1545 | 1.0059 | 0 |

### Filtering/ImageFusion（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `LabelOverlayImageFilter` | 15.8495 | 15.9594 | 1.0069 | 0 |
| `LabelToRGBImageFilter` | 15.5967 | 15.7334 | 1.0088 | 0 |

### Filtering/ImageLabel（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ChangeLabelImageFilter` | 6.8828 | 7.0324 | 1.0217 | 0 |
| `LabelContourImageFilter` | 15.4671 | 15.6567 | 1.0123 | 0 |

### Filtering/ImageSources（2）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `GridImageSource` | 19.341 | 22.5786 | 1.1674 | 0 |
| `PhysicalPointImageSource` | 7.2174 | 7.7164 | 1.0691 | 0 |

### Filtering/AnisotropicSmoothing（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `VectorCurvatureAnisotropicDiffusionImageFilter` | 306.5575 | 464.8061 | 1.5162 | 0 |

### Filtering/AntiAlias（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `AntiAliasBinaryImageFilter` | 182.0549 | 200.9727 | 1.1039 | 2.20e-07 |

### Filtering/BiasCorrection（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `N4BiasFieldCorrectionImageFilter` | 759.4252 | 767.7705 | 1.011 | 0 |

### Filtering/Colormap（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `ScalarToRGBColormapImageFilter` | 18.7654 | 26.3468 | 1.404 | 0 |

### Filtering/CurvatureFlow（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `BinaryMinMaxCurvatureFlowImageFilter` | 71.2831 | 85.4659 | 1.199 | 0 |

### Filtering/ImageFrequency（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `FrequencyBandImageFilter` | 8.5271 | 9.3543 | 1.097 | 0 |

### Filtering/SpatialFunction（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `SpatialFunctionImageEvaluatorFilter` | 0.0812 | 0.0853 | 1.0505 | 0 |

### Segmentation/KLMRegionGrowing（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `KLMRegionGrowImageFilter` | 1.0666 | 1.0667 | 1.0001 | 0 |

### Segmentation/MarkovRandomFieldsClassifiers（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `RGBGibbsPriorFilter` | 0.1869 | 0.1874 | 1.0031 | 0 |

### Segmentation/SuperPixel（1）

| 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | ---: | ---: | ---: | --- |
| `SLICImageFilter` | 1948.2156 | 1962.5268 | 1.0073 | 0 |

