# 灵昇 592 核：584 个接口函数混合精度盘点

本文按接口函数名逐条记录灵昇 592 核复测结果。数字来自 `mixed_precision_lingsheng592.csv`。
最终测试在灵昇计算节点完成（全量作业 1799867，累加开关 1800229，补测 1803449 / 1803421 / 1803853）。

混合精度：图像 float 存储对 double 存储。配准的度量、插值、优化器保持 double。
向量邻域按分量比误差；频率滤波按复数实部/虚部比误差。
合格：可判定的 `max_abs < 1e-5`，且 `speedup = double_ms / float_ms > 1`。
`qualified_functions.tsv` 有 **304** 条测试用例（含套件参数行和个别函数双 bench）。下表 **584** 行，一行一个函数名。

## 1. 总览

| 口径 | 个数 |
| --- | ---: |
| 接口函数 | 584 |
| 能做混合精度 | 471 |
|　误差且更快（表内合格） | 300 |
|　误差够、float 不快 | 142 |
|　float 更快、误差不够 | 27 |
|　误差不够且 float 不快 | 2 |
| 不能做混合精度 | 113 |

不能做的细分：

| 原因 | 个数 |
| --- | ---: |
| 基类空壳 | 75 |
| 整数标签二值 | 17 |
| 缺GPU | 10 |
| 缺FFTW | 9 |
| 缺FEM | 2 |

这 113 个全部是基类/空壳、整数/标签/二值、或缺 GPU/FFTW/FEM。
仍标失败的只有 `CurvatureRegistrationFilter`（缺 FFTW）。

## 2. 合格函数（误差 < 1e-5 且 float 更快）

共 **300** 个。计算节点跑法：`bash submit_592.sh`。

| 模块 | 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | --- | ---: | ---: | ---: | --- |
| Filtering/AnisotropicSmoothing | `VectorCurvatureAnisotropicDiffusionImageFilter` | 446.9178 | 528.8479 | 1.1833 | 0 |
| Filtering/AnisotropicSmoothing | `VectorGradientAnisotropicDiffusionImageFilter` | 447.0003 | 542.0450 | 1.2126 | 0 |
| Filtering/AntiAlias | `AntiAliasBinaryImageFilter` | 227.8046 | 255.6941 | 1.1224 | 0.00000022 |
| Filtering/BiasCorrection | `MRIBiasFieldCorrectionFilter` | 63.9780 | 64.2540 | 1.0043 | 0 |
| Filtering/BiasCorrection | `N4BiasFieldCorrectionImageFilter` | 853.6323 | 928.2977 | 1.0875 | 0.00000763 |
| Filtering/BinaryMathematicalMorphology | `BinaryClosingByReconstructionImageFilter` | 172.4956 | 187.1849 | 1.0852 | 0.00000000 |
| Filtering/BinaryMathematicalMorphology | `BinaryDilateImageFilter` | 81.8121 | 100.5649 | 1.2292 | 0.00000000 |
| Filtering/BinaryMathematicalMorphology | `BinaryMorphologicalClosingImageFilter` | 229.3171 | 238.3607 | 1.0394 | 0.00000000 |
| Filtering/BinaryMathematicalMorphology | `BinaryMorphologicalOpeningImageFilter` | 190.6664 | 210.5850 | 1.1045 | 0.00000000 |
| Filtering/BinaryMathematicalMorphology | `BinaryOpeningByReconstructionImageFilter` | 197.2310 | 211.8666 | 1.0742 | 0.00000000 |
| Filtering/BinaryMathematicalMorphology | `BinaryPruningImageFilter` | 174.5283 | 189.5027 | 1.0858 | 0.00000000 |
| Filtering/BinaryMathematicalMorphology | `FastIncrementalBinaryDilateImageFilter` | 85.4781 | 98.0968 | 1.1476 | 0.00000000 |
| Filtering/Colormap | `ScalarToRGBColormapImageFilter` | 24.5792 | 33.5728 | 1.3659 | 0.00000000 |
| Filtering/Convolution | `ConvolutionImageFilter` | 10.4936 | 21.8223 | 2.0796 | 0.00000732 |
| Filtering/Convolution | `FFTConvolutionImageFilter` | 433.6686 | 437.6057 | 1.0091 | 0.000008 |
| Filtering/Convolution | `FFTNormalizedCorrelationImageFilter` | 17769.9829 | 21400.5706 | 1.2043 | 0.00000328 |
| Filtering/Convolution | `MaskedFFTNormalizedCorrelationImageFilter` | 17825.8805 | 21333.4070 | 1.1968 | 0.00000036 |
| Filtering/Convolution | `NormalizedCorrelationImageFilter` | 8.6988 | 21.9182 | 2.5197 | 0.00000000 |
| Filtering/CurvatureFlow | `BinaryMinMaxCurvatureFlowImageFilter` | 89.9686 | 99.6556 | 1.1077 | 0.00000000 |
| Filtering/Deconvolution | `InverseDeconvolutionImageFilter` | 630.4134 | 675.3815 | 1.0713 | 0.00000000 |
| Filtering/Deconvolution | `LandweberDeconvolutionImageFilter` | 1566.5649 | 1657.8845 | 1.0583 | 0.00000000 |
| Filtering/Deconvolution | `ProjectedLandweberDeconvolutionImageFilter` | 1477.8455 | 1563.7859 | 1.0582 | 0.00000000 |
| Filtering/Deconvolution | `RichardsonLucyDeconvolutionImageFilter` | 2870.7802 | 2886.0829 | 1.0053 | 0.00000000 |
| Filtering/DiffusionTensorImage | `DiffusionTensor3DReconstructionImageFilter` | 147.6131 | 147.6536 | 1.0003 | 0 |
| Filtering/DisplacementField | `ComposeDisplacementFieldsImageFilter` | 10.4277 | 11.0878 | 1.0633 | 0.00000000 |
| Filtering/DisplacementField | `DisplacementFieldJacobianDeterminantFilter` | 18.6545 | 19.9939 | 1.0718 | 0.00000000 |
| Filtering/DisplacementField | `DisplacementFieldToBSplineImageFilter` | 36.9383 | 37.3177 | 1.0103 | 0 |
| Filtering/DisplacementField | `InvertDisplacementFieldImageFilter` | 117.7038 | 157.7151 | 1.3399 | 0.00000000 |
| Filtering/DisplacementField | `IterativeInverseDisplacementFieldImageFilter` | 2039.4140 | 2059.9331 | 1.0101 | 0 |
| Filtering/DistanceMap | `ApproximateSignedDistanceMapImageFilter` | 230.2142 | 276.2816 | 1.2001 | 0.00000024 |
| Filtering/DistanceMap | `ContourDirectedMeanDistanceImageFilter` | 69.7570 | 70.0787 | 1.0046 | 0.00000000 |
| Filtering/DistanceMap | `ContourMeanDistanceImageFilter` | 141.0003 | 144.5602 | 1.0252 | 0.00000000 |
| Filtering/DistanceMap | `FastChamferDistanceImageFilter` | 141.4137 | 163.9733 | 1.1595 | 0.00000000 |
| Filtering/DistanceMap | `HausdorffDistanceImageFilter` | 138.9430 | 141.9539 | 1.0217 | 0.00000000 |
| Filtering/DistanceMap | `IsoContourDistanceImageFilter` | 69.4480 | 71.0106 | 1.0225 | 0.00000001 |
| Filtering/FFT | `FFTPadImageFilter` | 7.3658 | 17.8280 | 2.4204 | 0.00000000 |
| Filtering/FFT | `FFTShiftImageFilter` | 7.5770 | 18.3491 | 2.4217 | 0.00000000 |
| Filtering/FFT | `FullToHalfHermitianImageFilter` | 9.1460 | 9.4221 | 1.0302 | 0 |
| Filtering/FFT | `HalfToFullHermitianImageFilter` | 9.8998 | 10.4431 | 1.0549 | 0 |
| Filtering/FFT | `VnlForward1DFFTImageFilter` | 10.3774 | 10.7697 | 1.0378 | 0 |
| Filtering/FFT | `VnlForwardFFTImageFilter` | 123.7489 | 185.7560 | 1.5011 | 0.00000000 |
| Filtering/FFT | `VnlHalfHermitianToRealInverseFFTImageFilter` | 115.0195 | 140.1699 | 1.2187 | 0 |
| Filtering/FFT | `VnlInverse1DFFTImageFilter` | 9.1802 | 9.2790 | 1.0108 | 0 |
| Filtering/FFT | `VnlInverseFFTImageFilter` | 215.7476 | 314.8236 | 1.4592 | 0.00000000 |
| Filtering/FFT | `VnlRealToHalfHermitianForwardFFTImageFilter` | 105.1463 | 124.5796 | 1.1848 | 0 |
| Filtering/ImageCompare | `AbsoluteValueDifferenceImageFilter` | 7.4346 | 18.2302 | 2.4521 | 0.00000000 |
| Filtering/ImageCompare | `CheckerBoardImageFilter` | 8.0038 | 20.8702 | 2.6075 | 0.00000000 |
| Filtering/ImageCompare | `SquaredDifferenceImageFilter` | 7.5413 | 18.0295 | 2.3908 | 0.00000000 |
| Filtering/ImageCompose | `ComposeImageFilter` | 8.6741 | 17.4328 | 2.0098 | 0.00000000 |
| Filtering/ImageCompose | `JoinImageFilter` | 9.5584 | 18.8556 | 1.9727 | 0.00000000 |
| Filtering/ImageCompose | `JoinSeriesImageFilter` | 2.9115 | 14.7034 | 5.0501 | 0.00000000 |
| Filtering/ImageFeature | `CannyEdgeDetectionImageFilter` | 71.8348 | 95.4221 | 1.3284 | 0.00000000 |
| Filtering/ImageFeature | `DerivativeImageFilter` | 7.8958 | 19.4418 | 2.4623 | 0.00000000 |
| Filtering/ImageFeature | `DiscreteGaussianDerivativeImageFilter` | 40.7438 | 64.0212 | 1.5713 | 0.00000045 |
| Filtering/ImageFeature | `GradientVectorFlowImageFilter` | 9.4031 | 9.7968 | 1.0419 | 0 |
| Filtering/ImageFeature | `HessianRecursiveGaussianImageFilter` | 132.7647 | 135.4944 | 1.0206 | 0.00000000 |
| Filtering/ImageFeature | `HessianToObjectnessMeasureImageFilter` | 154.6919 | 160.6145 | 1.0383 | 0 |
| Filtering/ImageFeature | `HoughTransform2DLinesImageFilter` | 2.8808 | 3.9542 | 1.3726 | 0 |
| Filtering/ImageFeature | `LaplacianImageFilter` | 7.8373 | 18.6396 | 2.3783 | 0.00000000 |
| Filtering/ImageFeature | `LaplacianRecursiveGaussianImageFilter` | 58.4374 | 74.2560 | 1.2707 | 0.00000000 |
| Filtering/ImageFeature | `LaplacianSharpeningImageFilter` | 45.3554 | 58.1203 | 1.2814 | 0.00000723 |
| Filtering/ImageFeature | `MultiScaleHessianBasedMeasureImageFilter` | 23.4170 | 26.2356 | 1.1204 | 0 |
| Filtering/ImageFeature | `SimpleContourExtractorImageFilter` | 8.1355 | 20.3427 | 2.5005 | 0.00000000 |
| Filtering/ImageFeature | `ZeroCrossingImageFilter` | 7.6271 | 18.8983 | 2.4778 | 0.00000000 |
| Filtering/ImageFilterBase | `NoiseImageFilter` | 8.9045 | 21.6545 | 2.4319 | 0.00000184 |
| Filtering/ImageFrequency | `FrequencyBandImageFilter` | 9.8075 | 10.0541 | 1.0251 | 0 |
| Filtering/ImageGradient | `DifferenceOfGaussiansGradientImageFilter` | 23.6543 | 57.1255 | 2.4150 | 0.00000000 |
| Filtering/ImageGradient | `GradientImageFilter` | 8.9578 | 21.5569 | 2.4065 | 0.00000000 |
| Filtering/ImageGradient | `GradientMagnitudeImageFilter` | 7.9659 | 19.1736 | 2.4070 | 0.00000188 |
| Filtering/ImageGradient | `GradientMagnitudeRecursiveGaussianImageFilter` | 1982.2408 | 2636.1558 | 1.3299 | 0.00000187 |
| Filtering/ImageGradient | `GradientRecursiveGaussianImageFilter` | 91.8006 | 3407.5553 | 37.1191 | 0.00000000 |
| Filtering/ImageGrid | `BSplineControlPointImageFilter` | 0.8381 | 0.8906 | 1.0626 | 0 |
| Filtering/ImageGrid | `BSplineDownsampleImageFilter` | 18.7473 | 54.2236 | 2.8923 | 0.00000000 |
| Filtering/ImageGrid | `BSplineScatteredDataPointSetToImageFilter` | 38.0584 | 38.9488 | 1.0234 | 0 |
| Filtering/ImageGrid | `BSplineUpsampleImageFilter` | 345.2311 | 365.6145 | 1.0590 | 0.00000000 |
| Filtering/ImageGrid | `BinShrinkImageFilter` | 8.7470 | 18.0664 | 2.0654 | 0.00000000 |
| Filtering/ImageGrid | `ChangeInformationImageFilter` | 0.0450 | 8.5694 | 190.4311 | 0.00000000 |
| Filtering/ImageGrid | `ConstantPadImageFilter` | 8.6668 | 17.5972 | 2.0304 | 0.00000000 |
| Filtering/ImageGrid | `CropImageFilter` | 9.3905 | 21.6690 | 2.3075 | 0.00000000 |
| Filtering/ImageGrid | `CyclicShiftImageFilter` | 8.8503 | 21.1452 | 2.3892 | 0.00000000 |
| Filtering/ImageGrid | `ExpandImageFilter` | 9.1716 | 25.2189 | 2.7497 | 0.00000000 |
| Filtering/ImageGrid | `FlipImageFilter` | 9.1125 | 21.1197 | 2.3177 | 0.00000000 |
| Filtering/ImageGrid | `InterpolateImageFilter` | 48.6846 | 51.2839 | 1.0534 | 0 |
| Filtering/ImageGrid | `MirrorPadImageFilter` | 9.1169 | 18.2265 | 1.9992 | 0.00000000 |
| Filtering/ImageGrid | `PasteImageFilter` | 8.3405 | 20.7980 | 2.4936 | 0.00000000 |
| Filtering/ImageGrid | `PermuteAxesImageFilter` | 8.2315 | 22.7487 | 2.7636 | 0.00000000 |
| Filtering/ImageGrid | `RegionOfInterestImageFilter` | 8.5773 | 19.6349 | 2.2892 | 0.00000000 |
| Filtering/ImageGrid | `ShrinkImageFilter` | 8.5085 | 17.4329 | 2.0489 | 0.00000000 |
| Filtering/ImageGrid | `SliceBySliceImageFilter` | 4.3444 | 4.7751 | 1.0991 | 0 |
| Filtering/ImageGrid | `TileImageFilter` | 21.5309 | 30.1458 | 1.4001 | 0.00000000 |
| Filtering/ImageGrid | `WarpImageFilter` | 9.5662 | 23.7943 | 2.4873 | 0.00000793 |
| Filtering/ImageGrid | `WrapPadImageFilter` | 9.3833 | 18.2912 | 1.9493 | 0.00000000 |
| Filtering/ImageGrid | `ZeroFluxNeumannPadImageFilter` | 9.6765 | 19.2186 | 1.9861 | 0.00000000 |
| Filtering/ImageIntensity | `AbsImageFilter` | 8.3682 | 18.7214 | 2.2372 | 0.00000000 |
| Filtering/ImageIntensity | `AcosImageFilter` | 8.0437 | 19.2230 | 2.3898 | 0.00000004 |
| Filtering/ImageIntensity | `AddImageFilter` | 8.1375 | 19.0878 | 2.3457 | 0.00000000 |
| Filtering/ImageIntensity | `AsinImageFilter` | 8.0426 | 18.5228 | 2.3031 | 0.00000004 |
| Filtering/ImageIntensity | `Atan2ImageFilter` | 7.5138 | 17.9489 | 2.3888 | 0.00000002 |
| Filtering/ImageIntensity | `AtanImageFilter` | 8.1553 | 19.3107 | 2.3679 | 0.00000006 |
| Filtering/ImageIntensity | `BoundedReciprocalImageFilter` | 7.2950 | 18.0952 | 2.4805 | 0.00000001 |
| Filtering/ImageIntensity | `ClampImageFilter` | 7.4014 | 19.2578 | 2.6019 | 0.00000000 |
| Filtering/ImageIntensity | `ComplexToImaginaryImageFilter` | 8.9714 | 8.9859 | 1.0016 | 0 |
| Filtering/ImageIntensity | `ComplexToModulusImageFilter` | 8.8746 | 8.9593 | 1.0095 | 0 |
| Filtering/ImageIntensity | `ComplexToPhaseImageFilter` | 8.6891 | 8.7020 | 1.0015 | 0 |
| Filtering/ImageIntensity | `ConstrainedValueAdditionImageFilter` | 8.1912 | 19.8509 | 2.4234 | 0.00000000 |
| Filtering/ImageIntensity | `ConstrainedValueDifferenceImageFilter` | 9.0345 | 17.7279 | 1.9622 | 0.00000000 |
| Filtering/ImageIntensity | `CosImageFilter` | 7.9765 | 31.9278 | 4.0027 | 0.00000003 |
| Filtering/ImageIntensity | `DivideOrZeroOutImageFilter` | 7.3454 | 17.4836 | 2.3802 | 0.00000000 |
| Filtering/ImageIntensity | `EdgePotentialImageFilter` | 18.8083 | 18.8685 | 1.0032 | 0 |
| Filtering/ImageIntensity | `ExpNegativeImageFilter` | 7.2813 | 17.9220 | 2.4614 | 0.00000001 |
| Filtering/ImageIntensity | `HistogramMatchingImageFilter` | 233.5931 | 267.2526 | 1.1441 | 0.00000000 |
| Filtering/ImageIntensity | `IntensityWindowingImageFilter` | 7.2992 | 18.8207 | 2.5785 | 0.00000000 |
| Filtering/ImageIntensity | `Log10ImageFilter` | 7.2581 | 19.4663 | 2.6820 | 0.00000012 |
| Filtering/ImageIntensity | `LogImageFilter` | 8.3660 | 21.2357 | 2.5383 | 0.00000024 |
| Filtering/ImageIntensity | `MagnitudeAndPhaseToComplexImageFilter` | 10.0964 | 10.6903 | 1.0588 | 0 |
| Filtering/ImageIntensity | `MaskImageFilter` | 9.1173 | 22.4769 | 2.4653 | 0.00000000 |
| Filtering/ImageIntensity | `MaskNegatedImageFilter` | 8.9512 | 22.2792 | 2.4890 | 0.00000000 |
| Filtering/ImageIntensity | `MaximumImageFilter` | 7.4402 | 17.8321 | 2.3967 | 0.00000000 |
| Filtering/ImageIntensity | `MinimumImageFilter` | 7.3640 | 17.4135 | 2.3647 | 0.00000000 |
| Filtering/ImageIntensity | `ModulusImageFilter` | 8.6851 | 8.6872 | 1.0002 | 0.00000000 |
| Filtering/ImageIntensity | `MultiplyImageFilter` | 7.4597 | 17.7185 | 2.3752 | 0.00000000 |
| Filtering/ImageIntensity | `NaryAddImageFilter` | 8.7524 | 17.2341 | 1.9691 | 0.00000000 |
| Filtering/ImageIntensity | `NaryMaximumImageFilter` | 8.7290 | 17.3953 | 1.9928 | 0.00000000 |
| Filtering/ImageIntensity | `NormalizeImageFilter` | 14.8045 | 25.0844 | 1.6944 | 0.00000006 |
| Filtering/ImageIntensity | `NormalizeToConstantImageFilter` | 14.6494 | 25.2819 | 1.7258 | 0.00000000 |
| Filtering/ImageIntensity | `NotImageFilter` | 8.6821 | 8.6913 | 1.0011 | 0.00000000 |
| Filtering/ImageIntensity | `RGBToLuminanceImageFilter` | 27.7237 | 28.0683 | 1.0124 | 0 |
| Filtering/ImageIntensity | `RescaleIntensityImageFilter` | 15.9219 | 34.7073 | 2.1798 | 0.00000003 |
| Filtering/ImageIntensity | `RoundImageFilter` | 7.4174 | 17.6608 | 2.3810 | 0.00000000 |
| Filtering/ImageIntensity | `ShiftScaleImageFilter` | 7.3253 | 17.7304 | 2.4204 | 0.00000000 |
| Filtering/ImageIntensity | `SinImageFilter` | 8.2551 | 19.7692 | 2.3948 | 0.00000003 |
| Filtering/ImageIntensity | `SqrtImageFilter` | 7.8557 | 18.4758 | 2.3519 | 0.00000047 |
| Filtering/ImageIntensity | `SquareImageFilter` | 7.3078 | 17.5473 | 2.4012 | 0.00000000 |
| Filtering/ImageIntensity | `SubtractImageFilter` | 7.4791 | 18.0608 | 2.4148 | 0.00000000 |
| Filtering/ImageIntensity | `TanImageFilter` | 8.3424 | 19.5682 | 2.3456 | 0.00000499 |
| Filtering/ImageIntensity | `TernaryAddImageFilter` | 8.6677 | 17.0858 | 1.9712 | 0.00000000 |
| Filtering/ImageIntensity | `TernaryMagnitudeImageFilter` | 8.7653 | 8.7654 | 1.0000 | 0 |
| Filtering/ImageIntensity | `VectorIndexSelectionCastImageFilter` | 8.8648 | 8.9805 | 1.0131 | 0 |
| Filtering/ImageIntensity | `VectorMagnitudeImageFilter` | 17.9442 | 27.9276 | 1.5564 | 0.00000000 |
| Filtering/ImageIntensity | `WeightedAddImageFilter` | 8.5839 | 20.9657 | 2.4424 | 0.00000000 |
| Filtering/ImageLabel | `LabelContourImageFilter` | 18.9164 | 19.1588 | 1.0128 | 0.00000000 |
| Filtering/ImageStatistics | `AccumulateImageFilter` | 26.9755 | 39.4028 | 1.4607 | 0.00000000 |
| Filtering/ImageStatistics | `AdaptiveHistogramEqualizationImageFilter` | 20.1254 | 34.0020 | 1.6895 | 0.00000763 |
| Filtering/ImageStatistics | `LabelStatisticsImageFilter` | 10.6719 | 10.7244 | 1.0049 | 0 |
| Filtering/ImageStatistics | `MaximumProjectionImageFilter` | 8.6680 | 18.0243 | 2.0794 | 0.00000000 |
| Filtering/ImageStatistics | `MeanProjectionImageFilter` | 8.7733 | 18.2993 | 2.0858 | 0.00000000 |
| Filtering/ImageStatistics | `MedianProjectionImageFilter` | 9.0085 | 19.1636 | 2.1273 | 0.00000000 |
| Filtering/ImageStatistics | `MinimumMaximumImageFilter` | 8.5332 | 16.9176 | 1.9826 | 0.00000000 |
| Filtering/ImageStatistics | `MinimumProjectionImageFilter` | 8.5881 | 17.9676 | 2.0922 | 0.00000000 |
| Filtering/ImageStatistics | `StandardDeviationProjectionImageFilter` | 8.8569 | 18.6400 | 2.1046 | 0.00000000 |
| Filtering/ImageStatistics | `StatisticsImageFilter` | 8.3897 | 17.0302 | 2.0299 | 0.00000000 |
| Filtering/ImageStatistics | `SumProjectionImageFilter` | 8.5962 | 18.2167 | 2.1192 | 0.00000000 |
| Filtering/LabelMap | `BinaryFillholeImageFilter` | 72.6172 | 72.9064 | 1.0040 | 0.00000000 |
| Filtering/LabelMap | `BinaryGrindPeakImageFilter` | 71.7226 | 71.8701 | 1.0021 | 0.00000000 |
| Filtering/LabelMap | `BinaryImageToLabelMapFilter` | 44.6171 | 45.8710 | 1.0281 | 0.00000000 |
| Filtering/LabelMap | `BinaryImageToShapeLabelMapFilter` | 65.8939 | 66.9205 | 1.0156 | 0.00000000 |
| Filtering/LabelMap | `BinaryImageToStatisticsLabelMapFilter` | 42.9732 | 52.6277 | 1.2247 | 0 |
| Filtering/LabelMap | `BinaryReconstructionByDilationImageFilter` | 49.2615 | 50.0462 | 1.0159 | 0 |
| Filtering/LabelMap | `BinaryReconstructionByErosionImageFilter` | 69.0219 | 70.2604 | 1.0179 | 0 |
| Filtering/LabelMap | `BinaryShapeKeepNObjectsImageFilter` | 70.6169 | 70.7558 | 1.0020 | 0.00000000 |
| Filtering/LabelMap | `BinaryStatisticsKeepNObjectsImageFilter` | 62.3374 | 68.0881 | 1.0923 | 0 |
| Filtering/LabelMap | `BinaryStatisticsOpeningImageFilter` | 62.2346 | 72.4117 | 1.1635 | 0 |
| Filtering/LabelMap | `ChangeLabelLabelMapFilter` | 46.2888 | 46.5087 | 1.0048 | 0.00000000 |
| Filtering/LabelMap | `LabelImageToLabelMapFilter` | 15.1163 | 15.3149 | 1.0131 | 0.00000000 |
| Filtering/LabelMap | `LabelImageToShapeLabelMapFilter` | 22.7938 | 22.8706 | 1.0034 | 0 |
| Filtering/LabelMap | `LabelMapMaskImageFilter` | 9.6202 | 16.4468 | 1.7096 | 0 |
| Filtering/LabelMap | `LabelShapeKeepNObjectsImageFilter` | 48.6980 | 55.3932 | 1.1375 | 0 |
| Filtering/LabelMap | `LabelStatisticsKeepNObjectsImageFilter` | 202.1349 | 214.6782 | 1.0621 | 0 |
| Filtering/LabelMap | `LabelStatisticsOpeningImageFilter` | 193.6011 | 218.8520 | 1.1304 | 0 |
| Filtering/LabelMap | `PadLabelMapFilter` | 0.1512 | 0.1523 | 1.0073 | 0 |
| Filtering/LabelMap | `ShapeRelabelImageFilter` | 79.6035 | 80.3075 | 1.0088 | 0.00000000 |
| Filtering/MathematicalMorphology | `BlackTopHatImageFilter` | 40.2532 | 54.7019 | 1.3589 | 0.00000000 |
| Filtering/MathematicalMorphology | `ClosingByReconstructionImageFilter` | 215.4166 | 328.3235 | 1.5241 | 0.00000000 |
| Filtering/MathematicalMorphology | `GrayscaleConnectedClosingImageFilter` | 780.5797 | 921.1825 | 1.1801 | 0 |
| Filtering/MathematicalMorphology | `GrayscaleConnectedOpeningImageFilter` | 1114.2579 | 1362.6894 | 1.2230 | 0 |
| Filtering/MathematicalMorphology | `GrayscaleDilateImageFilter` | 8.2555 | 19.0481 | 2.3073 | 0.00000000 |
| Filtering/MathematicalMorphology | `GrayscaleErodeImageFilter` | 8.1376 | 19.1019 | 2.3474 | 0.00000000 |
| Filtering/MathematicalMorphology | `GrayscaleFillholeImageFilter` | 580.8256 | 728.0557 | 1.2535 | 0.00000000 |
| Filtering/MathematicalMorphology | `GrayscaleGeodesicDilateImageFilter` | 53.1374 | 55.9961 | 1.0538 | 0 |
| Filtering/MathematicalMorphology | `GrayscaleGeodesicErodeImageFilter` | 55.7210 | 57.5508 | 1.0328 | 0 |
| Filtering/MathematicalMorphology | `GrayscaleGrindPeakImageFilter` | 315.1434 | 450.8954 | 1.4308 | 0.00000000 |
| Filtering/MathematicalMorphology | `GrayscaleMorphologicalClosingImageFilter` | 33.9758 | 49.0631 | 1.4441 | 0.00000000 |
| Filtering/MathematicalMorphology | `GrayscaleMorphologicalOpeningImageFilter` | 33.7075 | 49.1357 | 1.4577 | 0.00000000 |
| Filtering/MathematicalMorphology | `HConcaveImageFilter` | 237.8720 | 352.5932 | 1.4823 | 0.00000000 |
| Filtering/MathematicalMorphology | `HConvexImageFilter` | 274.2719 | 382.8168 | 1.3958 | 0.00000000 |
| Filtering/MathematicalMorphology | `HMaximaImageFilter` | 260.3618 | 370.2247 | 1.4220 | 0.00000000 |
| Filtering/MathematicalMorphology | `HMinimaImageFilter` | 237.2226 | 337.1781 | 1.4214 | 0.00000000 |
| Filtering/MathematicalMorphology | `MaskedMovingHistogramImageFilter` | 1.3348 | 3.0527 | 2.2870 | 0 |
| Filtering/MathematicalMorphology | `MorphologicalGradientImageFilter` | 24.0113 | 38.3034 | 1.5952 | 0.00000000 |
| Filtering/MathematicalMorphology | `MovingHistogramDilateImageFilter` | 11.0330 | 15.6033 | 1.4142 | 0 |
| Filtering/MathematicalMorphology | `MovingHistogramErodeImageFilter` | 15.0561 | 15.3907 | 1.0222 | 0 |
| Filtering/MathematicalMorphology | `MovingHistogramMorphologicalGradientImageFilter` | 10.4762 | 14.1540 | 1.3511 | 0 |
| Filtering/MathematicalMorphology | `OpeningByReconstructionImageFilter` | 208.2563 | 324.2597 | 1.5570 | 0.00000000 |
| Filtering/MathematicalMorphology | `RankImageFilter` | 10.9251 | 24.6886 | 2.2598 | 0.00000000 |
| Filtering/MathematicalMorphology | `ReconstructionByDilationImageFilter` | 239.0032 | 274.8895 | 1.1501 | 0 |
| Filtering/MathematicalMorphology | `ReconstructionByErosionImageFilter` | 229.3285 | 256.6733 | 1.1192 | 0 |
| Filtering/MathematicalMorphology | `RegionalMaximaImageFilter` | 180.4216 | 207.3117 | 1.1490 | 0.00000000 |
| Filtering/MathematicalMorphology | `RegionalMinimaImageFilter` | 180.3918 | 212.6674 | 1.1789 | 0.00000000 |
| Filtering/MathematicalMorphology | `ValuedRegionalMaximaImageFilter` | 174.9765 | 201.5951 | 1.1521 | 0.00000000 |
| Filtering/MathematicalMorphology | `ValuedRegionalMinimaImageFilter` | 166.8840 | 212.5533 | 1.2737 | 0.00000000 |
| Filtering/MathematicalMorphology | `VanHerkGilWermanErodeImageFilter` | 9.4311 | 9.6257 | 1.0206 | 0 |
| Filtering/MathematicalMorphology | `WhiteTopHatImageFilter` | 40.9018 | 58.8230 | 1.4382 | 0.00000000 |
| Filtering/Path | `ContourExtractor2DImageFilter` | 136.9034 | 168.6611 | 1.2320 | 0 |
| Filtering/Path | `ExtractOrthogonalSwath2DImageFilter` | 0.4419 | 8.6829 | 19.6490 | 0 |
| Filtering/Path | `PathToImageFilter` | 0.8676 | 0.9832 | 1.1332 | 0 |
| Filtering/QuadEdgeMeshFiltering | `BorderQuadEdgeMeshFilter` | 2.8975 | 2.9641 | 1.0230 | 0 |
| Filtering/QuadEdgeMeshFiltering | `DelaunayConformingQuadEdgeMeshFilter` | 27.8147 | 28.6729 | 1.0309 | 0 |
| Filtering/QuadEdgeMeshFiltering | `DiscreteGaussianCurvatureQuadEdgeMeshFilter` | 34.3402 | 34.6637 | 1.0094 | 0 |
| Filtering/QuadEdgeMeshFiltering | `DiscreteMaximumCurvatureQuadEdgeMeshFilter` | 43.4980 | 43.9011 | 1.0093 | 0 |
| Filtering/QuadEdgeMeshFiltering | `DiscreteMeanCurvatureQuadEdgeMeshFilter` | 41.8896 | 42.2882 | 1.0095 | 0 |
| Filtering/QuadEdgeMeshFiltering | `QuadricDecimationQuadEdgeMeshFilter` | 128.2714 | 128.4147 | 1.0011 | 0 |
| Filtering/QuadEdgeMeshFiltering | `SquaredEdgeLengthDecimationQuadEdgeMeshFilter` | 17.8739 | 17.8928 | 1.0011 | 0 |
| Filtering/Smoothing | `BinomialBlurImageFilter` | 131.2427 | 160.9334 | 1.2262 | 0.00000000 |
| Filtering/Smoothing | `BoxMeanImageFilter` | 63.4679 | 73.1032 | 1.1518 | 0.000008 |
| Filtering/Smoothing | `BoxSigmaImageFilter` | 11.4658 | 20.0018 | 1.7445 | 0.00000184 |
| Filtering/Smoothing | `FFTDiscreteGaussianImageFilter` | 641.6498 | 657.3299 | 1.0244 | 0.00000779 |
| Filtering/Smoothing | `MeanImageFilter` | 22.5230 | 57.9409 | 2.5725 | 0.000008 |
| Filtering/Smoothing | `MedianImageFilter` | 8.0778 | 18.7894 | 2.3261 | 0.00000000 |
| Filtering/Smoothing | `RecursiveGaussianImageFilter` | 8.8750 | 20.5673 | 2.3174 | 0.00000763 |
| Filtering/Smoothing | `SmoothingRecursiveGaussianImageFilter` | 648.1212 | 1982.3302 | 3.0586 | 0.00000000 |
| Filtering/SpatialFunction | `SpatialFunctionImageEvaluatorFilter` | 0.1112 | 0.1134 | 1.0198 | 0 |
| Filtering/Thresholding | `BinaryThresholdImageFilter` | 7.4313 | 17.8721 | 2.4050 | 0.00000000 |
| Filtering/Thresholding | `BinaryThresholdProjectionImageFilter` | 9.0626 | 18.9337 | 2.0892 | 0.00000000 |
| Filtering/Thresholding | `HistogramThresholdImageFilter` | 26.9941 | 40.8663 | 1.5139 | 0.00000000 |
| Filtering/Thresholding | `HuangThresholdImageFilter` | 26.2161 | 48.7965 | 1.8613 | 0.00000000 |
| Filtering/Thresholding | `IntermodesThresholdImageFilter` | 24.6150 | 36.8513 | 1.4971 | 0.00000000 |
| Filtering/Thresholding | `IsoDataThresholdImageFilter` | 25.3673 | 37.3288 | 1.4715 | 0.00000000 |
| Filtering/Thresholding | `KappaSigmaThresholdImageFilter` | 49.7379 | 94.8087 | 1.9062 | 0.00000000 |
| Filtering/Thresholding | `KittlerIllingworthThresholdImageFilter` | 24.2760 | 37.6775 | 1.5520 | 0.00000000 |
| Filtering/Thresholding | `LiThresholdImageFilter` | 24.5241 | 38.7595 | 1.5805 | 0.00000000 |
| Filtering/Thresholding | `MaximumEntropyThresholdImageFilter` | 26.9540 | 40.4248 | 1.4998 | 0.00000000 |
| Filtering/Thresholding | `MomentsThresholdImageFilter` | 24.2850 | 38.0722 | 1.5677 | 0.00000000 |
| Filtering/Thresholding | `OtsuMultipleThresholdsImageFilter` | 133.2390 | 172.8929 | 1.2976 | 0.00000000 |
| Filtering/Thresholding | `OtsuThresholdImageFilter` | 24.3005 | 36.9622 | 1.5210 | 0.00000000 |
| Filtering/Thresholding | `RenyiEntropyThresholdImageFilter` | 28.6875 | 42.7383 | 1.4898 | 0.00000000 |
| Filtering/Thresholding | `ShanbhagThresholdImageFilter` | 25.6424 | 39.1602 | 1.5272 | 0.00000000 |
| Filtering/Thresholding | `ThresholdImageFilter` | 8.8152 | 20.9739 | 2.3793 | 0.00000000 |
| Filtering/Thresholding | `ThresholdLabelerImageFilter` | 8.8734 | 22.9290 | 2.5840 | 0.00000000 |
| Filtering/Thresholding | `TriangleThresholdImageFilter` | 24.3923 | 39.2896 | 1.6107 | 0.00000000 |
| Filtering/Thresholding | `YenThresholdImageFilter` | 24.3923 | 38.5529 | 1.5805 | 0.00000000 |
| Registration/Common | `EuclideanDistancePointMetric` | 3444.3746 | 3445.7992 | 1.0004 | 0 |
| Registration/Common | `GradientDifferenceImageToImageMetric` | 2914.1278 | 4233.7450 | 1.4528 | 0 |
| Registration/Common | `KappaStatisticImageToImageMetric` | 205.0903 | 227.5513 | 1.1095 | 0 |
| Registration/Common | `MatchCardinalityImageToImageMetric` | 9.3484 | 9.9536 | 1.0647 | 0 |
| Registration/Common | `MattesMutualInformationImageToImageMetric` | 77.0138 | 91.5868 | 1.1892 | 0 |
| Registration/Common | `MultiResolutionImageRegistrationMethod` | 222.3121 | 234.6133 | 1.0553 | 0 |
| Registration/Common | `MutualInformationImageToImageMetric` | 24936.0921 | 24956.3895 | 1.0008 | 0 |
| Registration/Common | `PointSetToPointSetRegistrationMethod` | 130.9153 | 131.5314 | 1.0047 | 0 |
| Registration/Metricsv4 | `CorrelationImageToImageMetricv4` | 788.8065 | 1154.4573 | 1.4635 | 0 |
| Registration/Metricsv4 | `EuclideanDistancePointSetToPointSetMetricv4` | 39.7069 | 40.5640 | 1.0216 | 0 |
| Registration/Metricsv4 | `JensenHavrdaCharvatTsallisPointSetToPointSetMetricv4` | 32.4604 | 32.8881 | 1.0132 | 0 |
| Registration/Metricsv4 | `LabeledPointSetToPointSetMetricv4` | 0.3602 | 2.3471 | 6.5161 | 0 |
| Registration/Metricsv4 | `MattesMutualInformationImageToImageMetricv4` | 157.0261 | 162.8340 | 1.0370 | 0 |
| Registration/Metricsv4 | `ObjectToObjectMultiMetricv4` | 4911.9207 | 8092.1017 | 1.6474 | 0 |
| Registration/PDEDeformable | `DemonsRegistrationFilter` | 164.4006 | 192.0458 | 1.1682 | 0.00000000 |
| Registration/PDEDeformable | `DiffeomorphicDemonsRegistrationFilter` | 390.3208 | 430.1970 | 1.1022 | 0 |
| Registration/PDEDeformable | `FastSymmetricForcesDemonsRegistrationFilter` | 203.3162 | 241.7921 | 1.1892 | 0.00000000 |
| Registration/PDEDeformable | `LevelSetMotionRegistrationFilter` | 168.1427 | 202.9626 | 1.2071 | 0 |
| Registration/PDEDeformable | `SymmetricForcesDemonsRegistrationFilter` | 226.6369 | 264.3947 | 1.1666 | 0.00000000 |
| Registration/RegistrationMethodsv4 | `BSplineSyNImageRegistrationMethod` | 113.3329 | 116.9995 | 1.0324 | 6.69191785e-07 |
| Registration/RegistrationMethodsv4 | `ImageRegistrationMethodv4` | 55.9120 | 61.3057 | 1.0965 | 0 |
| Registration/RegistrationMethodsv4 | `TimeVaryingBSplineVelocityFieldImageRegistrationMethod` | 91.9560 | 196.0336 | 2.1318 | 6.69191785e-07 |
| Segmentation/Classifiers | `BayesianClassifierInitializationImageFilter` | 5154.8585 | 6441.5151 | 1.2496 | 0 |
| Segmentation/Classifiers | `ScalarImageKmeansImageFilter` | 4382.6583 | 5363.9316 | 1.2239 | 0.00000000 |
| Segmentation/ConnectedComponents | `HardConnectedComponentImageFilter` | 39.9148 | 44.2998 | 1.1099 | 0.00000000 |
| Segmentation/ConnectedComponents | `RelabelComponentImageFilter` | 16.8299 | 16.8445 | 1.0009 | 0.00000000 |
| Segmentation/ConnectedComponents | `ScalarConnectedComponentImageFilter` | 2651.5130 | 2711.3520 | 1.0226 | 0.00000000 |
| Segmentation/ConnectedComponents | `ThresholdMaximumConnectedComponentsImageFilter` | 1248.4535 | 1268.2009 | 1.0158 | 0.00000000 |
| Segmentation/DeformableMesh | `DeformableSimplexMesh3DBalloonForceFilter` | 20.7756 | 22.9768 | 1.1060 | 0 |
| Segmentation/DeformableMesh | `DeformableSimplexMesh3DGradientConstraintForceFilter` | 18.3695 | 20.0720 | 1.0927 | 0 |
| Segmentation/LabelVoting | `BinaryMedianImageFilter` | 8.5734 | 8.6852 | 1.0130 | 0.00000000 |
| Segmentation/LabelVoting | `VotingBinaryImageFilter` | 9.1785 | 9.2185 | 1.0044 | 0.00000000 |
| Segmentation/LevelSets | `AnisotropicFourthOrderLevelSetImageFilter` | 8714.2455 | 8848.1583 | 1.0154 | 0 |
| Segmentation/LevelSets | `BinaryMaskToNarrowBandPointSetFilter` | 165.3743 | 169.1982 | 1.0231 | 0 |
| Segmentation/LevelSets | `CannySegmentationLevelSetImageFilter` | 1354.2689 | 1388.2924 | 1.0251 | 0 |
| Segmentation/LevelSets | `CollidingFrontsImageFilter` | 4047.9856 | 4234.7684 | 1.0461 | 0 |
| Segmentation/LevelSets | `CurvesLevelSetImageFilter` | 275.1857 | 279.3746 | 1.0152 | 0 |
| Segmentation/LevelSets | `ExtensionVelocitiesImageFilter` | 1438.7623 | 1538.9544 | 1.0696 | 0 |
| Segmentation/LevelSets | `GeodesicActiveContourLevelSetImageFilter` | 274.2113 | 277.2320 | 1.0110 | 0 |
| Segmentation/LevelSets | `ImplicitManifoldNormalVectorFilter` | 102.9445 | 105.8714 | 1.0284 | 0 |
| Segmentation/LevelSets | `LaplacianSegmentationLevelSetImageFilter` | 99.7630 | 108.7787 | 1.0904 | 0 |
| Segmentation/LevelSets | `NarrowBandCurvesLevelSetImageFilter` | 516.7547 | 519.3602 | 1.0050 | 0 |
| Segmentation/LevelSets | `NarrowBandThresholdSegmentationLevelSetImageFilter` | 331.0552 | 352.9609 | 1.0662 | 0 |
| Segmentation/LevelSets | `ReinitializeLevelSetImageFilter` | 1267.7648 | 1315.8042 | 1.0379 | 0 |
| Segmentation/LevelSets | `ShapeDetectionLevelSetImageFilter` | 85.7215 | 85.8752 | 1.0018 | 0 |
| Segmentation/LevelSets | `ThresholdSegmentationLevelSetImageFilter` | 92.1627 | 113.3062 | 1.2294 | 0 |
| Segmentation/LevelSets | `UnsharpMaskLevelSetImageFilter` | 973.4904 | 992.5286 | 1.0196 | 0 |
| Segmentation/LevelSets | `VectorThresholdSegmentationLevelSetImageFilter` | 2.7548 | 3.2306 | 1.1727 | 0 |
| Segmentation/RegionGrowing | `ConfidenceConnectedImageFilter` | 6.3123 | 23.3247 | 3.6951 | 0.00000000 |
| Segmentation/RegionGrowing | `ConnectedThresholdImageFilter` | 1.4942 | 13.7897 | 9.2288 | 0.00000000 |
| Segmentation/RegionGrowing | `IsolatedConnectedImageFilter` | 17.8442 | 24.0244 | 1.3463 | 0.00000000 |
| Segmentation/RegionGrowing | `NeighborhoodConnectedImageFilter` | 3.0239 | 14.5993 | 4.8280 | 0.00000000 |
| Segmentation/RegionGrowing | `VectorConfidenceConnectedImageFilter` | 19.5894 | 36.6753 | 1.8722 | 0.00000000 |
| Segmentation/SuperPixel | `SLICImageFilter` | 2273.7936 | 2388.0833 | 1.0503 | 0.00000000 |
| Segmentation/Voronoi | `VoronoiPartitioningImageFilter` | 2.9730 | 2.9847 | 1.0039 | 0 |
| Segmentation/Voronoi | `VoronoiSegmentationImageFilter` | 2.5179 | 2.5605 | 1.0169 | 0 |
| Segmentation/Watersheds | `MorphologicalWatershedFromMarkersImageFilter` | 361.8832 | 420.7696 | 1.1627 | 0.00000000 |
| Segmentation/Watersheds | `MorphologicalWatershedImageFilter` | 1144.9824 | 1317.8210 | 1.1510 | 0.00000000 |
| Segmentation/Watersheds | `TobogganImageFilter` | 234.2334 | 265.1431 | 1.1320 | 0.00000000 |
| Segmentation/Watersheds | `WatershedImageFilter` | 927.1819 | 943.2518 | 1.0173 | 0.00000000 |

## 3. 能做但不合格

### 3.1 误差合格，float 不更快（142）

| 模块 | 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | --- | ---: | ---: | ---: | --- |
| Filtering/AnisotropicSmoothing | `CurvatureAnisotropicDiffusionImageFilter` | 4971.7569 | 4934.1439 | 0.9924 | 0.000008 |
| Filtering/AnisotropicSmoothing | `GradientAnisotropicDiffusionImageFilter` | 5393.7108 | 5368.9117 | 0.9954 | 0.000008 |
| Filtering/BinaryMathematicalMorphology | `BinaryThinningImageFilter` | 5.0874 | 5.0316 | 0.9890 | 0 |
| Filtering/BinaryMathematicalMorphology | `DilateObjectMorphologyImageFilter` | 12.5951 | 12.0231 | 0.9546 | 0.00000000 |
| Filtering/Deconvolution | `ParametricBlindLeastSquaresDeconvolutionImageFilter` | 15.9709 | 14.4017 | 0.9017 | 0 |
| Filtering/Deconvolution | `ProjectedIterativeDeconvolutionImageFilter` | 9.8634 | 8.6506 | 0.8770 | 0 |
| Filtering/Deconvolution | `TikhonovDeconvolutionImageFilter` | 655.3495 | 634.4370 | 0.9681 | 0.00000000 |
| Filtering/Deconvolution | `WienerDeconvolutionImageFilter` | 672.7202 | 670.5117 | 0.9967 | 0.00000000 |
| Filtering/Denoising | `PatchBasedDenoisingImageFilter` | 2889.1790 | 2299.0726 | 0.7958 | 0 |
| Filtering/DiffusionTensorImage | `TensorFractionalAnisotropyImageFilter` | 0.2379 | 0.2005 | 0.8428 | 0 |
| Filtering/DiffusionTensorImage | `TensorRelativeAnisotropyImageFilter` | 0.2348 | 0.1993 | 0.8488 | 0 |
| Filtering/DisplacementField | `ExponentialDisplacementFieldImageFilter` | 13.7599 | 13.1006 | 0.9521 | 0.00000000 |
| Filtering/DisplacementField | `InverseDisplacementFieldImageFilter` | 0.8158 | 0.7807 | 0.9570 | 0.00000000 |
| Filtering/DisplacementField | `TimeVaryingVelocityFieldIntegrationImageFilter` | 1.8713 | 0.6979 | 0.3729 | 0 |
| Filtering/DisplacementField | `TransformToDisplacementFieldFilter` | 8.5391 | 8.5345 | 0.9995 | 0 |
| Filtering/DistanceMap | `DirectedHausdorffDistanceImageFilter` | 72.7373 | 72.0422 | 0.9904 | 0.00000000 |
| Filtering/FFT | `VnlComplexToComplex1DFFTImageFilter` | 1.2372 | 0.8359 | 0.6756 | 0 |
| Filtering/FFT | `VnlComplexToComplexFFTImageFilter` | 1.4481 | 1.0504 | 0.7254 | 0 |
| Filtering/ImageCompare | `STAPLEImageFilter` | 82.2631 | 71.1567 | 0.8650 | 0.00000000 |
| Filtering/ImageCompare | `SimilarityIndexImageFilter` | 18.3475 | 16.1626 | 0.8809 | 0.00000000 |
| Filtering/ImageFeature | `Hessian3DToVesselnessMeasureImageFilter` | 14.4884 | 13.8649 | 0.9570 | 0 |
| Filtering/ImageFeature | `HoughTransform2DCirclesImageFilter` | 792.8947 | 678.2274 | 0.8554 | 0.00000000 |
| Filtering/ImageFeature | `MaskFeaturePointSelectionFilter` | 18.1158 | 18.0978 | 0.9990 | 0 |
| Filtering/ImageFeature | `UnsharpMaskImageFilter` | 652.4449 | 39.8447 | 0.0611 | 0.00000000 |
| Filtering/ImageFilterBase | `CastImageFilter` | 1.2990 | 0.8179 | 0.6296 | 0 |
| Filtering/ImageFilterBase | `VectorNeighborhoodOperatorImageFilter` | 10.2125 | 10.1658 | 0.9954 | 0.00000000e+00 |
| Filtering/ImageFusion | `LabelMapContourOverlayImageFilter` | 0.8675 | 0.8325 | 0.9597 | 0 |
| Filtering/ImageFusion | `LabelMapOverlayImageFilter` | 0.0960 | 0.0877 | 0.9135 | 0 |
| Filtering/ImageFusion | `LabelMapToRGBImageFilter` | 0.0439 | 0.0399 | 0.9089 | 0 |
| Filtering/ImageFusion | `LabelOverlayImageFilter` | 19.9764 | 19.7209 | 0.9872 | 0 |
| Filtering/ImageFusion | `LabelToRGBImageFilter` | 19.2490 | 18.9022 | 0.9820 | 0 |
| Filtering/ImageGradient | `VectorGradientMagnitudeImageFilter` | 1899.2526 | 1893.8403 | 0.9972 | 0 |
| Filtering/ImageGrid | `InterpolateImagePointsFilter` | 1.7629 | 1.2765 | 0.7241 | 0 |
| Filtering/ImageGrid | `OrientImageFilter` | 0.5196 | 0.4866 | 0.9365 | 0 |
| Filtering/ImageGrid | `PadImageFilter` | 7.8592 | 7.8421 | 0.9978 | 0 |
| Filtering/ImageGrid | `SliceImageFilter` | 8.6327 | 8.3810 | 0.9708 | 0 |
| Filtering/ImageGrid | `WarpVectorImageFilter` | 8.9750 | 8.9342 | 0.9955 | 0 |
| Filtering/ImageIntensity | `AndImageFilter` | 8.4276 | 8.2600 | 0.9801 | 0.00000000 |
| Filtering/ImageIntensity | `BinaryMagnitudeImageFilter` | 9.1687 | 9.0135 | 0.9831 | 0 |
| Filtering/ImageIntensity | `ComplexToRealImageFilter` | 9.0451 | 9.0286 | 0.9982 | 0 |
| Filtering/ImageIntensity | `MatrixIndexSelectionImageFilter` | 1.2473 | 0.8215 | 0.6586 | 0 |
| Filtering/ImageIntensity | `OrImageFilter` | 8.7054 | 8.6400 | 0.9925 | 0.00000000 |
| Filtering/ImageIntensity | `PolylineMask2DImageFilter` | 0.1203 | 0.1157 | 0.9618 | 0 |
| Filtering/ImageIntensity | `PolylineMaskImageFilter` | 0.6660 | 0.6615 | 0.9932 | 0 |
| Filtering/ImageIntensity | `SymmetricEigenAnalysisImageFilter` | 1.2861 | 0.8130 | 0.6321 | 0 |
| Filtering/ImageIntensity | `TernaryMagnitudeSquaredImageFilter` | 8.8920 | 8.4438 | 0.9496 | 0 |
| Filtering/ImageIntensity | `VectorExpandImageFilter` | 1.2990 | 0.9048 | 0.6965 | 0 |
| Filtering/ImageIntensity | `VectorRescaleIntensityImageFilter` | 14.0578 | 13.2485 | 0.9424 | 0 |
| Filtering/ImageIntensity | `XorImageFilter` | 8.9022 | 8.6937 | 0.9766 | 0.00000000 |
| Filtering/ImageLabel | `ChangeLabelImageFilter` | 8.6402 | 8.3584 | 0.9674 | 0.00000000 |
| Filtering/ImageSources | `GaborImageSource` | 98.4082 | 97.1485 | 0.9872 | 0.00000000 |
| Filtering/ImageSources | `GaussianImageSource` | 62.3841 | 60.5524 | 0.9706 | 0.00000000 |
| Filtering/ImageSources | `GridImageSource` | 24.8646 | 24.8564 | 0.9997 | 0 |
| Filtering/ImageSources | `PhysicalPointImageSource` | 8.7933 | 8.7859 | 0.9992 | 0 |
| Filtering/ImageStatistics | `BinaryProjectionImageFilter` | 8.5466 | 8.2172 | 0.9615 | 0.00000000 |
| Filtering/ImageStatistics | `GetAverageSliceImageFilter` | 0.4358 | 0.3733 | 0.8566 | 0 |
| Filtering/ImageStatistics | `LabelOverlapMeasuresImageFilter` | 9.1718 | 9.0989 | 0.9921 | 0 |
| Filtering/LabelMap | `AggregateLabelMapFilter` | 0.0367 | 0.0318 | 0.8665 | 0 |
| Filtering/LabelMap | `AutoCropLabelMapFilter` | 0.1445 | 0.1365 | 0.9446 | 0 |
| Filtering/LabelMap | `BinaryNotImageFilter` | 8.5764 | 8.3678 | 0.9757 | 0.00000000 |
| Filtering/LabelMap | `BinaryShapeOpeningImageFilter` | 51.7778 | 51.7031 | 0.9986 | 0 |
| Filtering/LabelMap | `ChangeRegionLabelMapFilter` | 0.0376 | 0.0324 | 0.8617 | 0 |
| Filtering/LabelMap | `ConvertLabelMapFilter` | 0.0352 | 0.0309 | 0.8778 | 0 |
| Filtering/LabelMap | `CropLabelMapFilter` | 0.0364 | 0.0342 | 0.9396 | 0 |
| Filtering/LabelMap | `LabelImageToStatisticsLabelMapFilter` | 278.7996 | 195.8250 | 0.7024 | 0 |
| Filtering/LabelMap | `LabelMapToAttributeImageFilter` | 0.0393 | 0.0362 | 0.9211 | 0 |
| Filtering/LabelMap | `LabelMapToBinaryImageFilter` | 80.7331 | 79.7473 | 0.9878 | 0.00000000 |
| Filtering/LabelMap | `LabelMapToLabelImageFilter` | 56.3464 | 55.4455 | 0.9840 | 0.00000000 |
| Filtering/LabelMap | `LabelSelectionLabelMapFilter` | 0.0437 | 0.0386 | 0.8833 | 0 |
| Filtering/LabelMap | `LabelShapeOpeningImageFilter` | 1665.9048 | 35.7107 | 0.0214 | 0 |
| Filtering/LabelMap | `MergeLabelMapFilter` | 0.0410 | 0.0377 | 0.9195 | 0 |
| Filtering/LabelMap | `RelabelLabelMapFilter` | 57.8726 | 56.6521 | 0.9789 | 0.00000000 |
| Filtering/LabelMap | `ShapeKeepNObjectsLabelMapFilter` | 66.8227 | 66.1002 | 0.9892 | 0.00000000 |
| Filtering/LabelMap | `ShapeLabelMapFilter` | 0.0373 | 0.0325 | 0.8713 | 0 |
| Filtering/LabelMap | `ShapeOpeningLabelMapFilter` | 0.0412 | 0.0373 | 0.9053 | 0 |
| Filtering/LabelMap | `ShapeRelabelLabelMapFilter` | 0.0358 | 0.0318 | 0.8883 | 0 |
| Filtering/LabelMap | `ShapeUniqueLabelMapFilter` | 0.0368 | 0.0318 | 0.8641 | 0 |
| Filtering/LabelMap | `ShiftScaleLabelMapFilter` | 0.0367 | 0.0343 | 0.9346 | 0 |
| Filtering/LabelMap | `StatisticsKeepNObjectsLabelMapFilter` | 0.0483 | 0.0435 | 0.9006 | 0 |
| Filtering/LabelMap | `StatisticsLabelMapFilter` | 0.0557 | 0.0504 | 0.9048 | 0 |
| Filtering/LabelMap | `StatisticsOpeningLabelMapFilter` | 0.0483 | 0.0450 | 0.9317 | 0 |
| Filtering/LabelMap | `StatisticsRelabelImageFilter` | 415.7889 | 213.2864 | 0.5130 | 0 |
| Filtering/LabelMap | `StatisticsRelabelLabelMapFilter` | 0.0370 | 0.0318 | 0.8595 | 0 |
| Filtering/MathematicalMorphology | `AnchorCloseImageFilter` | 27.9482 | 19.0432 | 0.6814 | 0 |
| Filtering/MathematicalMorphology | `AnchorDilateImageFilter` | 27.7843 | 18.4431 | 0.6638 | 0 |
| Filtering/MathematicalMorphology | `AnchorErodeImageFilter` | 20.1624 | 17.2946 | 0.8578 | 0 |
| Filtering/MathematicalMorphology | `AnchorOpenImageFilter` | 19.9330 | 19.3866 | 0.9726 | 0 |
| Filtering/MathematicalMorphology | `BasicDilateImageFilter` | 9.0052 | 8.8843 | 0.9866 | 0 |
| Filtering/MathematicalMorphology | `BasicErodeImageFilter` | 9.3250 | 9.2241 | 0.9892 | 0 |
| Filtering/MathematicalMorphology | `GrayscaleFunctionDilateImageFilter` | 9.3711 | 9.0641 | 0.9672 | 0 |
| Filtering/MathematicalMorphology | `GrayscaleFunctionErodeImageFilter` | 9.3304 | 9.3039 | 0.9972 | 0 |
| Filtering/MathematicalMorphology | `MaskedRankImageFilter` | 9.0616 | 8.9114 | 0.9834 | 0 |
| Filtering/MathematicalMorphology | `VanHerkGilWermanDilateImageFilter` | 9.5294 | 9.3903 | 0.9854 | 0 |
| Filtering/Path | `ChainCodeToFourierSeriesPathFilter` | 0.2403 | 0.2298 | 0.9563 | 0 |
| Filtering/Path | `OrthogonalSwath2DPathFilter` | 33.3044 | 33.2787 | 0.9992 | 0 |
| Filtering/Path | `PathToChainCodePathFilter` | 0.0690 | 0.0593 | 0.8594 | 0 |
| Filtering/QuadEdgeMeshFiltering | `CleanQuadEdgeMeshFilter` | 82.0696 | 79.4581 | 0.9682 | 0 |
| Filtering/QuadEdgeMeshFiltering | `DiscreteCurvatureTensorQuadEdgeMeshFilter` | 1.4187 | 1.3945 | 0.9829 | 0 |
| Filtering/QuadEdgeMeshFiltering | `DiscreteMinimumCurvatureQuadEdgeMeshFilter` | 44.2798 | 41.8767 | 0.9457 | 0 |
| Filtering/QuadEdgeMeshFiltering | `LaplacianDeformationQuadEdgeMeshFilterWithHardConstraints` | 23.2626 | 23.0654 | 0.9915 | 0 |
| Filtering/QuadEdgeMeshFiltering | `LaplacianDeformationQuadEdgeMeshFilterWithSoftConstraints` | 70.2938 | 66.6338 | 0.9479 | 0 |
| Filtering/QuadEdgeMeshFiltering | `NormalQuadEdgeMeshFilter` | 12.9288 | 12.7256 | 0.9843 | 0 |
| Filtering/QuadEdgeMeshFiltering | `ParameterizationQuadEdgeMeshFilter` | 5.2549 | 5.0129 | 0.9539 | 0 |
| Filtering/QuadEdgeMeshFiltering | `SmoothingQuadEdgeMeshFilter` | 35.5484 | 35.4992 | 0.9986 | 0 |
| Registration/Common | `BlockMatchingImageFilter` | 9.3957 | 9.2730 | 0.9869 | 0 |
| Registration/Common | `CorrelationCoefficientHistogramImageToImageMetric` | 3028.7069 | 451.8610 | 0.1492 | 0 |
| Registration/Common | `ImageRegistrationMethod` | 347.7772 | 298.0812 | 0.8571 | 0 |
| Registration/Common | `ImageToSpatialObjectRegistrationMethod` | 3.8127 | 3.3018 | 0.8660 | 0 |
| Registration/Common | `KullbackLeiblerCompareHistogramImageToImageMetric` | 2878.2035 | 781.7674 | 0.2716 | 0 |
| Registration/Common | `MeanReciprocalSquareDifferenceImageToImageMetric` | 2063.1428 | 151.2955 | 0.0733 | 0 |
| Registration/Common | `MeanReciprocalSquareDifferencePointSetToImageMetric` | 1333.3539 | 94.8270 | 0.0711 | 0 |
| Registration/Common | `MeanSquaresHistogramImageToImageMetric` | 1685.2272 | 449.5485 | 0.2668 | 0 |
| Registration/Common | `MeanSquaresImageToImageMetric` | 803.9038 | 182.8998 | 0.2275 | 0 |
| Registration/Common | `MeanSquaresPointSetToImageMetric` | 1999.7983 | 93.0553 | 0.0465 | 0 |
| Registration/Common | `MutualInformationHistogramImageToImageMetric` | 3026.7212 | 454.5267 | 0.1502 | 0 |
| Registration/Common | `NormalizedCorrelationImageToImageMetric` | 1397.5041 | 150.1957 | 0.1075 | 0 |
| Registration/Common | `NormalizedCorrelationPointSetToImageMetric` | 1356.4500 | 87.5187 | 0.0645 | 0 |
| Registration/Common | `NormalizedMutualInformationHistogramImageToImageMetric` | 2964.8902 | 447.7036 | 0.1510 | 0 |
| Registration/Common | `PointSetToImageRegistrationMethod` | 1989.2741 | 96.9736 | 0.0487 | 0 |
| Registration/Metricsv4 | `ANTSNeighborhoodCorrelationImageToImageMetricv4` | 1422.0842 | 99.6296 | 0.0701 | 0 |
| Registration/Metricsv4 | `DemonsImageToImageMetricv4` | 1365.4372 | 112.7249 | 0.0826 | 0 |
| Registration/Metricsv4 | `ExpectationBasedPointSetToPointSetMetricv4` | 10.4755 | 10.4693 | 0.9994 | 0 |
| Registration/Metricsv4 | `JointHistogramMutualInformationImageToImageMetricv4` | 830.3902 | 221.4213 | 0.2666 | 0 |
| Registration/Metricsv4 | `MeanSquaresImageToImageMetricv4` | 2662.8567 | 99.3655 | 0.0373 | 0 |
| Registration/RegistrationMethodsv4 | `SyNImageRegistrationMethod` | 73.8011 | 72.0185 | 0.9758 | 0 |
| Registration/RegistrationMethodsv4 | `TimeVaryingVelocityFieldImageRegistrationMethodv4` | 79.5106 | 63.3731 | 0.7970 | 0 |
| Segmentation/Classifiers | `BayesianClassifierImageFilter` | 12.9299 | 12.6031 | 0.9747 | 0 |
| Segmentation/ConnectedComponents | `ConnectedComponentImageFilter` | 45.0717 | 44.9351 | 0.9970 | 0.00000000 |
| Segmentation/ConnectedComponents | `VectorConnectedComponentImageFilter` | 0.2108 | 0.1897 | 0.8999 | 0 |
| Segmentation/DeformableMesh | `DeformableSimplexMesh3DFilter` | 63.5780 | 62.5379 | 0.9836 | 0 |
| Segmentation/KLMRegionGrowing | `KLMRegionGrowImageFilter` | 1.4254 | 1.4130 | 0.9913 | 0 |
| Segmentation/LabelVoting | `LabelVotingImageFilter` | 59.6722 | 59.2538 | 0.9930 | 0.00000000 |
| Segmentation/LabelVoting | `MultiLabelSTAPLEImageFilter` | 394.0115 | 217.3532 | 0.5516 | 0 |
| Segmentation/LabelVoting | `VotingBinaryHoleFillingImageFilter` | 7.8343 | 7.7510 | 0.9894 | 0.00000000 |
| Segmentation/LabelVoting | `VotingBinaryIterativeHoleFillingImageFilter` | 20.6600 | 19.0256 | 0.9209 | 0.00000000 |
| Segmentation/LevelSets | `GeodesicActiveContourShapePriorLevelSetImageFilter` | 20.5099 | 16.4425 | 0.8017 | 0 |
| Segmentation/LevelSets | `IsotropicFourthOrderLevelSetImageFilter` | 9394.8342 | 9142.8048 | 0.9732 | 0 |
| Segmentation/LevelSetsv4 | `LevelSetDomainMapImageFilter` | 0.1287 | 0.1152 | 0.8951 | 0 |
| Segmentation/MarkovRandomFieldsClassifiers | `MRFImageFilter` | 0.1666 | 0.1572 | 0.9433 | 0 |
| Segmentation/MarkovRandomFieldsClassifiers | `RGBGibbsPriorFilter` | 0.2678 | 0.2524 | 0.9423 | 0 |
| Segmentation/Voronoi | `VoronoiSegmentationRGBImageFilter` | 5.5239 | 5.1159 | 0.9261 | 0 |
| Segmentation/Watersheds | `IsolatedWatershedImageFilter` | 5857.0213 | 5600.4278 | 0.9562 | 0.00000000 |

### 3.2 float 更快，误差 ≥ 1e-5（27）

| 模块 | 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | --- | ---: | ---: | ---: | --- |
| Filtering/BinaryMathematicalMorphology | `BinaryErodeImageFilter` | 109.9556 | 129.8992 | 1.1814 | 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00000000 |
| Filtering/CurvatureFlow | `CurvatureFlowImageFilter` | 156.6581 | 176.5148 | 1.1268 | 0.0105 |
| Filtering/CurvatureFlow | `MinMaxCurvatureFlowImageFilter` | 144.4754 | 156.7361 | 1.0849 | 0.88824236 |
| Filtering/DistanceMap | `DanielssonDistanceMapImageFilter` | 392.2594 | 430.0120 | 1.0962 | 0.00001526 |
| Filtering/DistanceMap | `SignedDanielssonDistanceMapImageFilter` | 791.1480 | 805.7206 | 1.0184 | 0.00001526 |
| Filtering/FastMarching | `FastMarchingExtensionImageFilter` | 1313.5657 | 1389.2008 | 1.0576 | 0.00158745 |
| Filtering/FastMarching | `FastMarchingImageFilter` | 1145.1448 | 1154.3712 | 1.0081 | 0.00158745 |
| Filtering/FastMarching | `FastMarchingUpwindGradientImageFilter` | 1146.7602 | 1167.4117 | 1.0180 | 0.00158745 |
| Filtering/ImageFeature | `BilateralImageFilter` | 64.1074 | 77.7667 | 1.2131 | 0.000506352 |
| Filtering/ImageFeature | `SobelEdgeDetectionImageFilter` | 46.5266 | 63.7472 | 1.3701 | 0.00001523 |
| Filtering/ImageFeature | `ZeroCrossingBasedEdgeDetectionImageFilter` | 40.9003 | 56.0092 | 1.3694 | 1.00000000 |
| Filtering/ImageFrequency | `UnaryFrequencyDomainFilter` | 0.8562 | 1.0327 | 1.2062 | 7.69442780e-03 |
| Filtering/ImageGrid | `ResampleImageFilter` | 8.4368 | 19.8296 | 2.3504 | 0.0000 |
| Filtering/ImageIntensity | `DivideImageFilter` | 7.3571 | 17.6598 | 2.4004 | 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00000000 |
| Filtering/ImageIntensity | `ExpImageFilter` | 7.8331 | 19.0334 | 2.4299 | 1223427442060151630274911797248.00000000 |
| Filtering/ImageIntensity | `InvertIntensityImageFilter` | 7.2207 | 17.5264 | 2.4272 | 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00000000 |
| Filtering/ImageIntensity | `PowImageFilter` | 8.6203 | 19.8579 | 2.3036 | 82005602291106567229987094528.00000000 |
| Filtering/ImageIntensity | `SigmoidImageFilter` | 7.4409 | 18.6796 | 2.5104 | 179769272747276226945882377411290981404325996661361720468412242522537928070797565271909689592602339599350907611450162626401899105572463550995096731772245729323459178378228860588246212067815818137070158542464648878681654516295341964992555331502414606404604810361088786013481013344477275997335941101361335107584.00000000 |
| Filtering/ImageLabel | `BinaryContourImageFilter` | 19.6836 | 19.7709 | 1.0044 | 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00000000 |
| Filtering/ImageNoise | `AdditiveGaussianNoiseImageFilter` | 8.4770 | 20.7103 | 2.4431 | 0.00001500 |
| Filtering/ImageNoise | `SaltAndPepperNoiseImageFilter` | 8.0725 | 20.5733 | 2.5486 | 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00000000 |
| Filtering/ImageNoise | `ShotNoiseImageFilter` | 8.4521 | 21.7286 | 2.5708 | 0.00001526 |
| Filtering/ImageNoise | `SpeckleNoiseImageFilter` | 21.4203 | 33.7828 | 1.5771 | 0.00001525 |
| Filtering/MathematicalMorphology | `DoubleThresholdImageFilter` | 198.2277 | 291.2158 | 1.4691 | 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.00000000 |
| Filtering/Smoothing | `DiscreteGaussianImageFilter` | 18.9865 | 35.4388 | 1.8665 | 0.000013 |
| Registration/Common | `MultiResolutionPyramidImageFilter` | 69.4316 | 84.3880 | 1.2154 | 0.00001883 |
| Registration/Common | `RecursiveMultiResolutionPyramidImageFilter` | 40.6553 | 57.3018 | 1.4095 | 0.00001498 |

### 3.3 误差不够且 float 不更快（2）

| 模块 | 函数 | float_ms | double_ms | 加速比 | max_abs |
| --- | --- | ---: | ---: | ---: | --- |
| Filtering/BinaryMathematicalMorphology | `ErodeObjectMorphologyImageFilter` | 12.0276 | 11.7084 | 0.9735 | 1.00000000 |
| Filtering/DistanceMap | `SignedMaurerDistanceMapImageFilter` | 65.7250 | 59.0155 | 0.8979 | 0.00001526 |

## 4. 不能做混合精度（113）

派生类多数已在第 2、3 节测过。下列接口不能单独形成可比的 float vs double 输出。

### 4.1 基类空壳（75）

不能单独实例化，或必须再挂 functor / 结构元素 / 路径 / 水平集速度场。

| 模块 | 函数 | 灵昇复测 | 备注摘要 |
| --- | --- | --- | --- |
| Filtering/AnisotropicSmoothing | `AnisotropicDiffusionImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/BinaryMathematicalMorphology | `ObjectMorphologyImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/Convolution | `ConvolutionImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/Deconvolution | `IterativeDeconvolutionImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/Denoising | `PatchBasedDenoisingBaseImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `ComplexToComplex1DFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `ComplexToComplexFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `Forward1DFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `ForwardFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `HalfHermitianToRealInverseFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `Inverse1DFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `InverseFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FFT | `RealToHalfHermitianForwardFFTImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FastMarching | `FastMarchingExtensionImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FastMarching | `FastMarchingImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FastMarching | `FastMarchingQuadEdgeMeshFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/FastMarching | `FastMarchingUpwindGradientImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageFilterBase | `BinaryFunctorImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `BinaryGeneratorImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `BoxImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `KernelImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `MaskNeighborhoodOperatorImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `MovingHistogramImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `MovingHistogramImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageFilterBase | `NeighborhoodOperatorImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `RecursiveSeparableImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageFilterBase | `TernaryFunctorImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `TernaryGeneratorImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageFilterBase | `UnaryGeneratorImageFilter` | 未测 | 这是框架/基类，要再挂具体规则才能跑。不能单独做混合精度对比。 |
| Filtering/ImageGrid | `BSplineCenteredL2ResampleImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageGrid | `BSplineCenteredResampleImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageGrid | `BSplineL2ResampleImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageGrid | `BSplineResampleImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageGrid | `PadImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageIntensity | `AdaptImageFilter` | 未测 | 需要掩膜或第二张图，单张切片不够。未测混合精度。 |
| Filtering/ImageIntensity | `NaryFunctorImageFilter` | 未测 | 需要掩膜或第二张图，单张切片不够。未测混合精度。 |
| Filtering/ImageIntensity | `TernaryOperatorImageFilter` | 未测 | 需要掩膜或第二张图，单张切片不够。未测混合精度。 |
| Filtering/ImageNoise | `NoiseBaseImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageSources | `GenerateImageSource` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageSources | `ParametricImageSource` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/ImageStatistics | `ProjectionImageFilter` | 未测 | 结果往往是一个统计量，或把整张图压成一条/一列，不是整张逐像素灰度图。混合精度要比两张同尺寸亮度图的差，这里对不上。 |
| Filtering/MathematicalMorphology | `AnchorErodeDilateImageFilter` | 未测 | 需要特定形状的结构元素；同类灰度形态学已测一批，本函数未纳入同一套混合精度对比。 |
| Filtering/MathematicalMorphology | `AnchorOpenCloseImageFilter` | 未测 | 需要特定形状的结构元素；同类灰度形态学已测一批，本函数未纳入同一套混合精度对比。 |
| Filtering/MathematicalMorphology | `MorphologyImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/MathematicalMorphology | `MovingHistogramMorphologyImageFilter` | 未测 | 需要特定形状的结构元素；同类灰度形态学已测一批，本函数未纳入同一套混合精度对比。 |
| Filtering/MathematicalMorphology | `ReconstructionImageFilter` | 未测 | 需要特定形状的结构元素；同类灰度形态学已测一批，本函数未纳入同一套混合精度对比。 |
| Filtering/MathematicalMorphology | `ValuedRegionalExtremaImageFilter` | 未测 | 需要特定形状的结构元素；同类灰度形态学已测一批，本函数未纳入同一套混合精度对比。 |
| Filtering/MathematicalMorphology | `VanHerkGilWermanErodeDilateImageFilter` | 未测 | 需要特定形状的结构元素；同类灰度形态学已测一批，本函数未纳入同一套混合精度对比。 |
| Filtering/Path | `ImageAndPathToImageFilter` | 未测 | 处理的是曲线/折线，不是一张图片。没有逐像素亮度可对比，故不能做混合精度。 |
| Filtering/Path | `ImageToPathFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/Path | `PathAndImageToPathFilter` | 未测 | 处理的是曲线/折线，不是一张图片。没有逐像素亮度可对比，故不能做混合精度。 |
| Filtering/Path | `PathToPathFilter` | 未测 | 处理的是曲线/折线，不是一张图片。没有逐像素亮度可对比，故不能做混合精度。 |
| Filtering/QuadEdgeMeshFiltering | `DecimationQuadEdgeMeshFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/QuadEdgeMeshFiltering | `DiscreteCurvatureQuadEdgeMeshFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/QuadEdgeMeshFiltering | `DiscretePrincipalCurvaturesQuadEdgeMeshFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/QuadEdgeMeshFiltering | `EdgeDecimationQuadEdgeMeshFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Filtering/QuadEdgeMeshFiltering | `LaplacianDeformationQuadEdgeMeshFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Common | `CompareHistogramImageToImageMetric` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Common | `HistogramImageToImageMetric` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Common | `ImageToImageMetric` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Common | `ImageToSpatialObjectMetric` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Common | `PointSetToImageMetric` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Common | `PointSetToPointSetMetric` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Metricsv4 | `ImageToImageMetricv4` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Metricsv4 | `PointSetToPointSetMetricWithIndexv4` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/Metricsv4 | `PointSetToPointSetMetricv4` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Registration/PDEDeformable | `PDEDeformableRegistrationFilter` | 未测 | 需要两张图反复对齐，不是单张图滤波。本次未按这套输入测混合精度。 |
| Segmentation/KLMRegionGrowing | `RegionGrowImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Segmentation/LevelSets | `NarrowBandLevelSetImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Segmentation/LevelSets | `ParallelSparseFieldLevelSetImageFilter` | 未测 | 缺完整输入（速度场、形状模型或差分函数），不能单独当一张灰度滤波来比混合精度。 |
| Segmentation/LevelSets | `SegmentationLevelSetImageFilter` | 未测 | 缺完整输入（速度场、形状模型或差分函数），不能单独当一张灰度滤波来比混合精度。 |
| Segmentation/LevelSets | `ShapePriorSegmentationLevelSetImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Segmentation/LevelSets | `SparseFieldFourthOrderLevelSetImageFilter` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |
| Segmentation/LevelSets | `SparseFieldLevelSetImageFilter` | 未测 | 缺完整输入（速度场、形状模型或差分函数），不能单独当一张灰度滤波来比混合精度。 |
| Segmentation/Voronoi | `VoronoiSegmentationImageFilterBase` | 未测 | 这是空壳接口，不能单独拿一张图计时。没有可比较的输入输出，所以不能做混合精度。 |

### 4.2 整数标签二值（17）

输出是编号或 0/1，不是连续灰度。

| 模块 | 函数 | 灵昇复测 | 备注摘要 |
| --- | --- | --- | --- |
| Filtering/BinaryMathematicalMorphology | `BinaryMorphologyImageFilter` | 未测 | 主要输出 0 和 1（物体/背景），不是连续灰度。表上的混合精度专指灰度图单精度 vs 双精度，故不填。 |
| Filtering/LabelMap | `AttributeKeepNObjectsLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `AttributeOpeningLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `AttributePositionLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `AttributeRelabelLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `AttributeSelectionLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `AttributeUniqueLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `BinaryReconstructionLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `InPlaceLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `LabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `LabelUniqueLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `ObjectByObjectLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `RegionFromReferenceLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `ShapePositionLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `StatisticsPositionLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Filtering/LabelMap | `StatisticsUniqueLabelMapFilter` | 未测 | 输出是编号表（这块是 1、那块是 2），不是亮度图。混合精度要比的是同一套亮度用单精度和双精度各算一遍；门牌号不必算到小数，所以不填。 |
| Segmentation/ConnectedComponents | `ConnectedComponentFunctorImageFilter` | 未测 | 输出是整数标签或分区编号，不是连续亮度。换成单精度/双精度没有「更亮一点」可对比，故不填混合精度。 |

### 4.3 缺GPU（10）

GPU 滤波器，本次 ITK 未开 GPU。

| 模块 | 函数 | 灵昇复测 | 备注摘要 |
| --- | --- | --- | --- |
| Filtering/GPUAnisotropicSmoothing | `GPUAnisotropicDiffusionImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Filtering/GPUAnisotropicSmoothing | `GPUGradientAnisotropicDiffusionImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Filtering/GPUImageFilterBase | `GPUBoxImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Filtering/GPUImageFilterBase | `GPUCastImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Filtering/GPUImageFilterBase | `GPUNeighborhoodOperatorImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Filtering/GPUSmoothing | `GPUDiscreteGaussianImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Filtering/GPUSmoothing | `GPUMeanImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Filtering/GPUThresholding | `GPUBinaryThresholdImageFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Registration/GPUPDEDeformable | `GPUDemonsRegistrationFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |
| Registration/GPUPDEDeformable | `GPUPDEDeformableRegistrationFilter` | 未测 | 这是给显卡用的函数。鲲鹏这次没开显卡，跑不了，所以没有混合精度时间。 |

### 4.4 缺FFTW（9）

依赖 FFTW。当前库未定义 `ITK_USE_FFTWF/D`，机器无 libfftw。同类 Vnl FFT 已测。

| 模块 | 函数 | 灵昇复测 | 备注摘要 |
| --- | --- | --- | --- |
| Filtering/FFT | `FFTWComplexToComplex1DFFTImageFilter` | 未测 | 每个像素不是一个亮度，而是彩色、箭头（位移）或多个数。混合精度对比需要一张灰度亮度图，套不上。 |
| Filtering/FFT | `FFTWComplexToComplexFFTImageFilter` | 未测 | 每个像素不是一个亮度，而是彩色、箭头（位移）或多个数。混合精度对比需要一张灰度亮度图，套不上。 |
| Filtering/FFT | `FFTWForward1DFFTImageFilter` | 未测 | 本机没用这套傅里叶变换库，跑不了，所以没有混合精度时间。 |
| Filtering/FFT | `FFTWForwardFFTImageFilter` | 未测 | 本机没用这套傅里叶变换库，跑不了，所以没有混合精度时间。 |
| Filtering/FFT | `FFTWHalfHermitianToRealInverseFFTImageFilter` | 未测 | 本机没用这套傅里叶变换库，跑不了，所以没有混合精度时间。 |
| Filtering/FFT | `FFTWInverse1DFFTImageFilter` | 未测 | 本机没用这套傅里叶变换库，跑不了，所以没有混合精度时间。 |
| Filtering/FFT | `FFTWInverseFFTImageFilter` | 未测 | 本机没用这套傅里叶变换库，跑不了，所以没有混合精度时间。 |
| Filtering/FFT | `FFTWRealToHalfHermitianForwardFFTImageFilter` | 未测 | 本机没用这套傅里叶变换库，跑不了，所以没有混合精度时间。 |
| Registration/PDEDeformable | `CurvatureRegistrationFilter` | 失败 | 需要两张图反复对齐，不是单张图滤波。本次未按这套输入测混合精度。 灵昇 592 核复测没有有效对比：needs_FFTW。 灵昇 592 核补测没有有效对比：needs_FF... |

### 4.5 缺FEM（2）

FEM 模块未编译，需要力学网格，不是亮度滤波。

| 模块 | 函数 | 灵昇复测 | 备注摘要 |
| --- | --- | --- | --- |
| Registration/FEM | `FEMRegistrationFilter` | 未测 | 需要力学网格和材料参数，不是对一张亮度图滤波。不能做混合精度。 |
| Registration/FEM | `PhysicsBasedNonRigidRegistrationMethod` | 未测 | 需要力学网格和材料参数，不是对一张亮度图滤波。不能做混合精度。 |

## 5. 按模块统计

| 模块 | 合计 | 合格 | 能做不合格 | 不能做 |
| --- | ---: | ---: | ---: | ---: |
| Filtering/AnisotropicSmoothing | 5 | 2 | 2 | 1 |
| Filtering/AntiAlias | 1 | 1 | 0 | 0 |
| Filtering/BiasCorrection | 2 | 2 | 0 | 0 |
| Filtering/BinaryMathematicalMorphology | 13 | 7 | 4 | 2 |
| Filtering/Colormap | 1 | 1 | 0 | 0 |
| Filtering/Convolution | 6 | 5 | 0 | 1 |
| Filtering/CurvatureFlow | 3 | 1 | 2 | 0 |
| Filtering/Deconvolution | 9 | 4 | 4 | 1 |
| Filtering/Denoising | 2 | 0 | 1 | 1 |
| Filtering/DiffusionTensorImage | 3 | 1 | 2 | 0 |
| Filtering/DisplacementField | 9 | 5 | 4 | 0 |
| Filtering/DistanceMap | 10 | 6 | 4 | 0 |
| Filtering/FFT | 28 | 10 | 2 | 16 |
| Filtering/FastMarching | 7 | 0 | 3 | 4 |
| Filtering/GPUAnisotropicSmoothing | 2 | 0 | 0 | 2 |
| Filtering/GPUImageFilterBase | 3 | 0 | 0 | 3 |
| Filtering/GPUSmoothing | 2 | 0 | 0 | 2 |
| Filtering/GPUThresholding | 1 | 0 | 0 | 1 |
| Filtering/ImageCompare | 5 | 3 | 2 | 0 |
| Filtering/ImageCompose | 3 | 3 | 0 | 0 |
| Filtering/ImageFeature | 20 | 13 | 7 | 0 |
| Filtering/ImageFilterBase | 15 | 1 | 2 | 12 |
| Filtering/ImageFrequency | 2 | 1 | 1 | 0 |
| Filtering/ImageFusion | 5 | 0 | 5 | 0 |
| Filtering/ImageGradient | 6 | 5 | 1 | 0 |
| Filtering/ImageGrid | 33 | 22 | 6 | 5 |
| Filtering/ImageIntensity | 67 | 47 | 17 | 3 |
| Filtering/ImageLabel | 3 | 1 | 2 | 0 |
| Filtering/ImageNoise | 5 | 0 | 4 | 1 |
| Filtering/ImageSources | 6 | 0 | 4 | 2 |
| Filtering/ImageStatistics | 15 | 11 | 3 | 1 |
| Filtering/LabelMap | 60 | 19 | 26 | 15 |
| Filtering/MathematicalMorphology | 49 | 31 | 11 | 7 |
| Filtering/Path | 10 | 3 | 3 | 4 |
| Filtering/QuadEdgeMeshFiltering | 20 | 7 | 8 | 5 |
| Filtering/Smoothing | 9 | 8 | 1 | 0 |
| Filtering/SpatialFunction | 1 | 1 | 0 | 0 |
| Filtering/Thresholding | 19 | 19 | 0 | 0 |
| Registration/Common | 31 | 8 | 17 | 6 |
| Registration/FEM | 2 | 0 | 0 | 2 |
| Registration/GPUPDEDeformable | 2 | 0 | 0 | 2 |
| Registration/Metricsv4 | 14 | 6 | 5 | 3 |
| Registration/PDEDeformable | 7 | 5 | 0 | 2 |
| Registration/RegistrationMethodsv4 | 5 | 3 | 2 | 0 |
| Segmentation/Classifiers | 3 | 2 | 1 | 0 |
| Segmentation/ConnectedComponents | 7 | 4 | 2 | 1 |
| Segmentation/DeformableMesh | 3 | 2 | 1 | 0 |
| Segmentation/KLMRegionGrowing | 2 | 0 | 1 | 1 |
| Segmentation/LabelVoting | 6 | 2 | 4 | 0 |
| Segmentation/LevelSets | 24 | 16 | 2 | 6 |
| Segmentation/LevelSetsv4 | 1 | 0 | 1 | 0 |
| Segmentation/MarkovRandomFieldsClassifiers | 2 | 0 | 2 | 0 |
| Segmentation/RegionGrowing | 5 | 5 | 0 | 0 |
| Segmentation/SuperPixel | 1 | 1 | 0 | 0 |
| Segmentation/Voronoi | 4 | 2 | 1 | 1 |
| Segmentation/Watersheds | 5 | 4 | 1 | 0 |

## 6. 数据与作业

- 主测图：`test/data/BrainProtonDensity1024.png`。
- 配准对：`BrainProtonDensity1024_fixed.png` / `_moving.png`。
- 小图补测：细化/Hough 64×64；MRF 6×6×3；Gibbs 20×20×1；网格 RegularSphere。
- 作业 1803853：向量邻域、UnaryFrequency、BSplineSyN、TimeVaryingBSpline。后两个已进合格名单。

不要把 SSH 密码写进仓库。不要在登录节点跑 `run_qualified.sh`。

