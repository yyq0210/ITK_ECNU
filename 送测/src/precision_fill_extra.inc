// auto-generated extra 2D filter benches
  Run2<itk::HuangThresholdImageFilter<FImg, FImg>, itk::HuangThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "HuangThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::IntermodesThresholdImageFilter<FImg, FImg>, itk::IntermodesThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "IntermodesThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::IsoDataThresholdImageFilter<FImg, FImg>, itk::IsoDataThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "IsoDataThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::KittlerIllingworthThresholdImageFilter<FImg, FImg>, itk::KittlerIllingworthThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "KittlerIllingworthThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::LiThresholdImageFilter<FImg, FImg>, itk::LiThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "LiThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::MaximumEntropyThresholdImageFilter<FImg, FImg>, itk::MaximumEntropyThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "MaximumEntropyThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::MomentsThresholdImageFilter<FImg, FImg>, itk::MomentsThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "MomentsThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::RenyiEntropyThresholdImageFilter<FImg, FImg>, itk::RenyiEntropyThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "RenyiEntropyThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::ShanbhagThresholdImageFilter<FImg, FImg>, itk::ShanbhagThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "ShanbhagThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::TriangleThresholdImageFilter<FImg, FImg>, itk::TriangleThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "TriangleThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::YenThresholdImageFilter<FImg, FImg>, itk::YenThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "YenThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::OtsuMultipleThresholdsImageFilter<FImg, FImg>, itk::OtsuMultipleThresholdsImageFilter<DImg, DImg>>(
    "Thresholding", "OtsuMultipleThresholdsImageFilter", input, runs,
    [&](auto * f) { f->SetNumberOfThresholds(2); },
    [&](auto * f) { f->SetNumberOfThresholds(2); });
  Run2<itk::ClampImageFilter<FImg, FImg>, itk::ClampImageFilter<DImg, DImg>>(
    "ImageIntensity", "ClampImageFilter", input, runs,
    [&](auto * f) { f->SetBounds(0.0f, 255.0f); },
    [&](auto * f) { f->SetBounds(0.0, 255.0); });
  Run2<itk::SigmoidImageFilter<FImg, FImg>, itk::SigmoidImageFilter<DImg, DImg>>(
    "ImageIntensity", "SigmoidImageFilter", input, runs,
    [&](auto * f) { f->SetAlpha(10.0); f->SetBeta(100.0); },
    [&](auto * f) { f->SetAlpha(10.0); f->SetBeta(100.0); });
  Run2<itk::IntensityWindowingImageFilter<FImg, FImg>, itk::IntensityWindowingImageFilter<DImg, DImg>>(
    "ImageIntensity", "IntensityWindowingImageFilter", input, runs,
    [&](auto * f) { f->SetWindowMinimum(0.0f); f->SetWindowMaximum(255.0f); },
    [&](auto * f) { f->SetWindowMinimum(0.0); f->SetWindowMaximum(255.0); });
  Run2<itk::NormalizeToConstantImageFilter<FImg, FImg>, itk::NormalizeToConstantImageFilter<DImg, DImg>>(
    "ImageIntensity", "NormalizeToConstantImageFilter", input, runs,
    [&](auto * f) { f->SetConstant(1.0f); },
    [&](auto * f) { f->SetConstant(1.0); });
  Run2<itk::Log10ImageFilter<FImg, FImg>, itk::Log10ImageFilter<DImg, DImg>>(
    "ImageIntensity", "Log10ImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::LogImageFilter<FImg, FImg>, itk::LogImageFilter<DImg, DImg>>(
    "ImageIntensity", "LogImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::BinomialBlurImageFilter<FImg, FImg>, itk::BinomialBlurImageFilter<DImg, DImg>>(
    "Smoothing", "BinomialBlurImageFilter", input, runs,
    [&](auto * f) { f->SetRepetitions(2); },
    [&](auto * f) { f->SetRepetitions(2); });
  Run2<itk::UnsharpMaskImageFilter<FImg, FImg>, itk::UnsharpMaskImageFilter<DImg, DImg>>(
    "ImageFeature", "UnsharpMaskImageFilter", input, runs,
    [&](auto * f) { f->SetAmount(0.5); },
    [&](auto * f) { f->SetAmount(0.5); });
  Run2<itk::ZeroCrossingBasedEdgeDetectionImageFilter<FImg, FImg>, itk::ZeroCrossingBasedEdgeDetectionImageFilter<DImg, DImg>>(
    "ImageFeature", "ZeroCrossingBasedEdgeDetectionImageFilter", input, runs,
    [&](auto * f) { f->SetVariance(2.0); },
    [&](auto * f) { f->SetVariance(2.0); });
  Run2<itk::DerivativeImageFilter<FImg, FImg>, itk::DerivativeImageFilter<DImg, DImg>>(
    "ImageFeature", "DerivativeImageFilter", input, runs,
    [&](auto * f) { f->SetOrder(1); f->SetDirection(0); },
    [&](auto * f) { f->SetOrder(1); f->SetDirection(0); });
  Run2<itk::DiscreteGaussianDerivativeImageFilter<FImg, FImg>, itk::DiscreteGaussianDerivativeImageFilter<DImg, DImg>>(
    "ImageFeature", "DiscreteGaussianDerivativeImageFilter", input, runs,
    [&](auto * f) { f->SetVariance(2.0); },
    [&](auto * f) { f->SetVariance(2.0); });
  Run2<itk::CannyEdgeDetectionImageFilter<FImg, FImg>, itk::CannyEdgeDetectionImageFilter<DImg, DImg>>(
    "ImageFeature", "CannyEdgeDetectionImageFilter", input, runs,
    [&](auto * f) { f->SetVariance(2.0); f->SetUpperThreshold(10.0); f->SetLowerThreshold(5.0); },
    [&](auto * f) { f->SetVariance(2.0); f->SetUpperThreshold(10.0); f->SetLowerThreshold(5.0); });
  Run2<itk::SimpleContourExtractorImageFilter<FImg, FImg>, itk::SimpleContourExtractorImageFilter<DImg, DImg>>(
    "ImageFeature", "SimpleContourExtractorImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::GradientMagnitudeRecursiveGaussianImageFilter<FImg, FImg>, itk::GradientMagnitudeRecursiveGaussianImageFilter<DImg, DImg>>(
    "ImageGradient", "GradientMagnitudeRecursiveGaussianImageFilter", input, runs,
    [&](auto * f) { f->SetSigma(1.5); },
    [&](auto * f) { f->SetSigma(1.5); });
  Run1<itk::FlipImageFilter<FImg>, itk::FlipImageFilter<DImg>>(
    "ImageGrid", "FlipImageFilter", input, runs,
    [&](auto * f) { auto a=f->GetFlipAxes(); a[0]=true; f->SetFlipAxes(a); },
    [&](auto * f) { auto a=f->GetFlipAxes(); a[0]=true; f->SetFlipAxes(a); });
  Run2<itk::ShrinkImageFilter<FImg, FImg>, itk::ShrinkImageFilter<DImg, DImg>>(
    "ImageGrid", "ShrinkImageFilter", input, runs,
    [&](auto * f) { f->SetShrinkFactors(2); },
    [&](auto * f) { f->SetShrinkFactors(2); });
  Run2<itk::ExpandImageFilter<FImg, FImg>, itk::ExpandImageFilter<DImg, DImg>>(
    "ImageGrid", "ExpandImageFilter", input, runs,
    [&](auto * f) { f->SetExpandFactors(2); },
    [&](auto * f) { f->SetExpandFactors(2); });
  Run2<itk::ConstantPadImageFilter<FImg, FImg>, itk::ConstantPadImageFilter<DImg, DImg>>(
    "ImageGrid", "ConstantPadImageFilter", input, runs,
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); f->SetConstant(0.0f); },
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); f->SetConstant(0.0); });
  Run2<itk::MirrorPadImageFilter<FImg, FImg>, itk::MirrorPadImageFilter<DImg, DImg>>(
    "ImageGrid", "MirrorPadImageFilter", input, runs,
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); },
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); });
  Run2<itk::WrapPadImageFilter<FImg, FImg>, itk::WrapPadImageFilter<DImg, DImg>>(
    "ImageGrid", "WrapPadImageFilter", input, runs,
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); },
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); });
  Run2<itk::ZeroFluxNeumannPadImageFilter<FImg, FImg>, itk::ZeroFluxNeumannPadImageFilter<DImg, DImg>>(
    "ImageGrid", "ZeroFluxNeumannPadImageFilter", input, runs,
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); },
    [&](auto * f) { f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); });
  Run2<itk::CropImageFilter<FImg, FImg>, itk::CropImageFilter<DImg, DImg>>(
    "ImageGrid", "CropImageFilter", input, runs,
    [&](auto * f) { f->SetLowerBoundaryCropSize(pad); f->SetUpperBoundaryCropSize(pad); },
    [&](auto * f) { f->SetLowerBoundaryCropSize(pad); f->SetUpperBoundaryCropSize(pad); });
  Run1<itk::CyclicShiftImageFilter<FImg>, itk::CyclicShiftImageFilter<DImg>>(
    "ImageGrid", "CyclicShiftImageFilter", input, runs,
    [&](auto * f) { itk::Offset<2> s; s.Fill(5); f->SetShift(s); },
    [&](auto * f) { itk::Offset<2> s; s.Fill(5); f->SetShift(s); });
  Run2<itk::BinShrinkImageFilter<FImg, FImg>, itk::BinShrinkImageFilter<DImg, DImg>>(
    "ImageGrid", "BinShrinkImageFilter", input, runs,
    [&](auto * f) { f->SetShrinkFactors(2); },
    [&](auto * f) { f->SetShrinkFactors(2); });
  Run1<itk::ChangeInformationImageFilter<FImg>, itk::ChangeInformationImageFilter<DImg>>(
    "ImageGrid", "ChangeInformationImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::RegionOfInterestImageFilter<FImg, FImg>, itk::RegionOfInterestImageFilter<DImg, DImg>>(
    "ImageGrid", "RegionOfInterestImageFilter", input, runs,
    [&](auto * f) { f->SetRegionOfInterest(roi); },
    [&](auto * f) { f->SetRegionOfInterest(roi); });
  Run2<itk::AdditiveGaussianNoiseImageFilter<FImg, FImg>, itk::AdditiveGaussianNoiseImageFilter<DImg, DImg>>(
    "ImageNoise", "AdditiveGaussianNoiseImageFilter", input, runs,
    [&](auto * f) { f->SetStandardDeviation(5.0); f->SetSeed(1); },
    [&](auto * f) { f->SetStandardDeviation(5.0); f->SetSeed(1); });
  Run2<itk::SaltAndPepperNoiseImageFilter<FImg, FImg>, itk::SaltAndPepperNoiseImageFilter<DImg, DImg>>(
    "ImageNoise", "SaltAndPepperNoiseImageFilter", input, runs,
    [&](auto * f) { f->SetProbability(0.01); f->SetSeed(1); },
    [&](auto * f) { f->SetProbability(0.01); f->SetSeed(1); });
  Run2<itk::ShotNoiseImageFilter<FImg, FImg>, itk::ShotNoiseImageFilter<DImg, DImg>>(
    "ImageNoise", "ShotNoiseImageFilter", input, runs,
    [&](auto * f) { f->SetScale(1.0); f->SetSeed(1); },
    [&](auto * f) { f->SetScale(1.0); f->SetSeed(1); });
  Run2<itk::SpeckleNoiseImageFilter<FImg, FImg>, itk::SpeckleNoiseImageFilter<DImg, DImg>>(
    "ImageNoise", "SpeckleNoiseImageFilter", input, runs,
    [&](auto * f) { f->SetStandardDeviation(0.1); f->SetSeed(1); },
    [&](auto * f) { f->SetStandardDeviation(0.1); f->SetSeed(1); });
  Run2<itk::HMaximaImageFilter<FImg, FImg>, itk::HMaximaImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "HMaximaImageFilter", input, runs,
    [&](auto * f) { f->SetHeight(10.0); },
    [&](auto * f) { f->SetHeight(10.0); });
  Run2<itk::HMinimaImageFilter<FImg, FImg>, itk::HMinimaImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "HMinimaImageFilter", input, runs,
    [&](auto * f) { f->SetHeight(10.0); },
    [&](auto * f) { f->SetHeight(10.0); });
  Run2<itk::HConvexImageFilter<FImg, FImg>, itk::HConvexImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "HConvexImageFilter", input, runs,
    [&](auto * f) { f->SetHeight(10.0); },
    [&](auto * f) { f->SetHeight(10.0); });
  Run2<itk::HConcaveImageFilter<FImg, FImg>, itk::HConcaveImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "HConcaveImageFilter", input, runs,
    [&](auto * f) { f->SetHeight(10.0); },
    [&](auto * f) { f->SetHeight(10.0); });
  Run2<itk::RegionalMaximaImageFilter<FImg, FImg>, itk::RegionalMaximaImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "RegionalMaximaImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::RegionalMinimaImageFilter<FImg, FImg>, itk::RegionalMinimaImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "RegionalMinimaImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::GrayscaleFillholeImageFilter<FImg, FImg>, itk::GrayscaleFillholeImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "GrayscaleFillholeImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::GrayscaleGrindPeakImageFilter<FImg, FImg>, itk::GrayscaleGrindPeakImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "GrayscaleGrindPeakImageFilter", input, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  Run2<itk::RankImageFilter<FImg, FImg>, itk::RankImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "RankImageFilter", input, runs,
    [&](auto * f) { f->SetRadius(radius); },
    [&](auto * f) { f->SetRadius(radius); });
  RunMorph<itk::OpeningByReconstructionImageFilter<FImg, FImg, BallF>, itk::OpeningByReconstructionImageFilter<DImg, DImg, BallD>>(
    "MathematicalMorphology", "OpeningByReconstructionImageFilter", input, kf, kd, runs);
  RunMorph<itk::ClosingByReconstructionImageFilter<FImg, FImg, BallF>, itk::ClosingByReconstructionImageFilter<DImg, DImg, BallD>>(
    "MathematicalMorphology", "ClosingByReconstructionImageFilter", input, kf, kd, runs);
  RunBinMorph<itk::BinaryOpeningByReconstructionImageFilter<FImg, BallF>, itk::BinaryOpeningByReconstructionImageFilter<DImg, BallD>>(
    "BinaryMathematicalMorphology", "BinaryOpeningByReconstructionImageFilter", binF, binD, kf, kd, runs);
  RunBinMorph<itk::BinaryClosingByReconstructionImageFilter<FImg, BallF>, itk::BinaryClosingByReconstructionImageFilter<DImg, BallD>>(
    "BinaryMathematicalMorphology", "BinaryClosingByReconstructionImageFilter", binF, binD, kf, kd, runs);
  RunBin1<itk::BinaryFillholeImageFilter<FImg>, itk::BinaryFillholeImageFilter<DImg>>(
    "BinaryMathematicalMorphology", "BinaryFillholeImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetForegroundValue(1.0f); },
    [&](auto * f) { f->SetForegroundValue(1.0); });
  RunBin<itk::BinaryPruningImageFilter<FImg, FImg>, itk::BinaryPruningImageFilter<DImg, DImg>>(
    "BinaryMathematicalMorphology", "BinaryPruningImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetIteration(3); },
    [&](auto * f) { f->SetIteration(3); });
  RunBin<itk::BinaryThinningImageFilter<FImg, FImg>, itk::BinaryThinningImageFilter<DImg, DImg>>(
    "BinaryMathematicalMorphology", "BinaryThinningImageFilter", binF, binD, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  RunBin<itk::ApproximateSignedDistanceMapImageFilter<FImg, FImg>, itk::ApproximateSignedDistanceMapImageFilter<DImg, DImg>>(
    "DistanceMap", "ApproximateSignedDistanceMapImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  RunBin<itk::FastChamferDistanceImageFilter<FImg, FImg>, itk::FastChamferDistanceImageFilter<DImg, DImg>>(
    "DistanceMap", "FastChamferDistanceImageFilter", binF, binD, runs,
    [&](auto * f) { (void)0; },
    [&](auto * f) { (void)0; });
  RunBin<itk::IsoContourDistanceImageFilter<FImg, FImg>, itk::IsoContourDistanceImageFilter<DImg, DImg>>(
    "DistanceMap", "IsoContourDistanceImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetLevelSetValue(0.5); },
    [&](auto * f) { f->SetLevelSetValue(0.5); });
  RunBin<itk::BinaryContourImageFilter<FImg, FImg>, itk::BinaryContourImageFilter<DImg, DImg>>(
    "ImageLabel", "BinaryContourImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetForegroundValue(1.0f); },
    [&](auto * f) { f->SetForegroundValue(1.0); });
  Run1<itk::AdaptiveHistogramEqualizationImageFilter<FImg>, itk::AdaptiveHistogramEqualizationImageFilter<DImg>>(
    "ImageStatistics", "AdaptiveHistogramEqualizationImageFilter", input, runs,
    [&](auto * f) { f->SetRadius(radius); f->SetAlpha(0.3); f->SetBeta(0.3); },
    [&](auto * f) { f->SetRadius(radius); f->SetAlpha(0.3); f->SetBeta(0.3); });
  Run2<itk::MinMaxCurvatureFlowImageFilter<FImg, FImg>, itk::MinMaxCurvatureFlowImageFilter<DImg, DImg>>(
    "CurvatureFlow", "MinMaxCurvatureFlowImageFilter", input, runs,
    [&](auto * f) { f->SetTimeStep(0.125); f->SetNumberOfIterations(6); f->SetStencilRadius(1); },
    [&](auto * f) { f->SetTimeStep(0.125); f->SetNumberOfIterations(6); f->SetStencilRadius(1); });
  RunBin<itk::AntiAliasBinaryImageFilter<FImg, FImg>, itk::AntiAliasBinaryImageFilter<DImg, DImg>>(
    "AntiAlias", "AntiAliasBinaryImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetMaximumRMSError(0.01); f->SetNumberOfIterations(6); },
    [&](auto * f) { f->SetMaximumRMSError(0.01); f->SetNumberOfIterations(6); });
  Run2<itk::ConnectedThresholdImageFilter<FImg, FImg>, itk::ConnectedThresholdImageFilter<DImg, DImg>>(
    "RegionGrowing", "ConnectedThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetSeed(seed); f->SetLower(50.0f); f->SetUpper(200.0f); f->SetReplaceValue(1.0f); },
    [&](auto * f) { f->SetSeed(seed); f->SetLower(50.0); f->SetUpper(200.0); f->SetReplaceValue(1.0); });
  Run2<itk::ConfidenceConnectedImageFilter<FImg, FImg>, itk::ConfidenceConnectedImageFilter<DImg, DImg>>(
    "RegionGrowing", "ConfidenceConnectedImageFilter", input, runs,
    [&](auto * f) { f->SetSeed(seed); f->SetMultiplier(2.5); f->SetNumberOfIterations(2); f->SetReplaceValue(1.0f); },
    [&](auto * f) { f->SetSeed(seed); f->SetMultiplier(2.5); f->SetNumberOfIterations(2); f->SetReplaceValue(1.0); });
  Run2<itk::NeighborhoodConnectedImageFilter<FImg, FImg>, itk::NeighborhoodConnectedImageFilter<DImg, DImg>>(
    "RegionGrowing", "NeighborhoodConnectedImageFilter", input, runs,
    [&](auto * f) { f->SetSeed(seed); f->SetLower(50.0f); f->SetUpper(200.0f); f->SetRadius(radius); f->SetReplaceValue(1.0f); },
    [&](auto * f) { f->SetSeed(seed); f->SetLower(50.0); f->SetUpper(200.0); f->SetRadius(radius); f->SetReplaceValue(1.0); });
  RunBin<itk::BinaryMedianImageFilter<FImg, FImg>, itk::BinaryMedianImageFilter<DImg, DImg>>(
    "LabelVoting", "BinaryMedianImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetRadius(radius); },
    [&](auto * f) { f->SetRadius(radius); });
  RunBin<itk::VotingBinaryImageFilter<FImg, FImg>, itk::VotingBinaryImageFilter<DImg, DImg>>(
    "LabelVoting", "VotingBinaryImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetRadius(radius); f->SetForegroundValue(1.0f); f->SetBackgroundValue(0.0f); },
    [&](auto * f) { f->SetRadius(radius); f->SetForegroundValue(1.0); f->SetBackgroundValue(0.0); });
  RunBin<itk::VotingBinaryHoleFillingImageFilter<FImg, FImg>, itk::VotingBinaryHoleFillingImageFilter<DImg, DImg>>(
    "LabelVoting", "VotingBinaryHoleFillingImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetRadius(radius); f->SetForegroundValue(1.0f); f->SetBackgroundValue(0.0f); },
    [&](auto * f) { f->SetRadius(radius); f->SetForegroundValue(1.0); f->SetBackgroundValue(0.0); });
  Run2<itk::MultiResolutionPyramidImageFilter<FImg, FImg>, itk::MultiResolutionPyramidImageFilter<DImg, DImg>>(
    "Registration", "MultiResolutionPyramidImageFilter", input, runs,
    [&](auto * f) { f->SetNumberOfLevels(2); },
    [&](auto * f) { f->SetNumberOfLevels(2); });
  Run2<itk::RecursiveMultiResolutionPyramidImageFilter<FImg, FImg>, itk::RecursiveMultiResolutionPyramidImageFilter<DImg, DImg>>(
    "Registration", "RecursiveMultiResolutionPyramidImageFilter", input, runs,
    [&](auto * f) { f->SetNumberOfLevels(2); },
    [&](auto * f) { f->SetNumberOfLevels(2); });
