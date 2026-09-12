// 造数据第三批：位移场/距离/反卷积/FFT/投影/投票/分水岭等。串行不要写这里。
{
  using VFImg = itk::Image<itk::Vector<float, 2>, Dim>;
  using VDImg = itk::Image<itk::Vector<double, 2>, Dim>;
  using U16Img = itk::Image<unsigned short, Dim>;
  using CImgF = itk::Image<std::complex<float>, Dim>;
  using CImgD = itk::Image<std::complex<double>, Dim>;

  VFImg::Pointer dispF2;
  VDImg::Pointer dispD2;
  FImg::Pointer  kernelF3;
  DImg::Pointer  kernelD3;
  U16Img::Pointer lab16b;
  if (!g_list)
  {
    using Gsrc = itk::GaussianImageSource<FImg>;
    auto gs = Gsrc::New();
    FImg::SizeType ksz;
    ksz.Fill(9);
    gs->SetSize(ksz);
    gs->SetNormalized(true);
    gs->SetSigma(1.5);
    gs->Update();
    kernelF3 = gs->GetOutput();
    kernelF3->DisconnectPipeline();
    using CK = itk::CastImageFilter<FImg, DImg>;
    auto ck = CK::New();
    ck->SetInput(kernelF3);
    ck->Update();
    kernelD3 = ck->GetOutput();
    kernelD3->DisconnectPipeline();
    auto cc0 = itk::ConnectedComponentImageFilter<FImg, U16Img>::New();
    cc0->SetInput(binF);
    cc0->Update();
    lab16b = cc0->GetOutput();
    lab16b->DisconnectPipeline();
    dispF2 = VFImg::New();
    dispF2->CopyInformation(input);
    dispF2->SetRegions(input->GetLargestPossibleRegion());
    dispF2->Allocate();
    itk::Vector<float, 2> zf;
    zf[0] = 0.2f;
    zf[1] = 0.1f;
    dispF2->FillBuffer(zf);
    dispD2 = VDImg::New();
    dispD2->CopyInformation(input);
    dispD2->SetRegions(input->GetLargestPossibleRegion());
    dispD2->Allocate();
    itk::Vector<double, 2> zd;
    zd[0] = 0.2;
    zd[1] = 0.1;
    dispD2->FillBuffer(zd);
  }

  BINARY_SELF("ImageIntensity", ConstrainedValueDifferenceImageFilter);
  Bench(
    "ImageIntensity",
    "TernaryAddImageFilter",
    [&]() {
      auto f = itk::TernaryAddImageFilter<FImg, FImg, FImg, FImg>::New();
      f->SetInput1(input);
      f->SetInput2(input);
      f->SetInput3(input);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::TernaryAddImageFilter<DImg, DImg, DImg, DImg>::New();
      f->SetInput1(c->GetOutput());
      f->SetInput2(c->GetOutput());
      f->SetInput3(c->GetOutput());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  Bench(
    "ImageIntensity",
    "NaryAddImageFilter",
    [&]() {
      auto f = itk::NaryAddImageFilter<FImg, FImg>::New();
      f->SetInput(0, input);
      f->SetInput(1, input);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::NaryAddImageFilter<DImg, DImg>::New();
      f->SetInput(0, c->GetOutput());
      f->SetInput(1, c->GetOutput());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "ImageIntensity",
    "NaryMaximumImageFilter",
    [&]() {
      auto f = itk::NaryMaximumImageFilter<FImg, FImg>::New();
      f->SetInput(0, input);
      f->SetInput(1, input);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::NaryMaximumImageFilter<DImg, DImg>::New();
      f->SetInput(0, c->GetOutput());
      f->SetInput(1, c->GetOutput());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  Run2<itk::DoubleThresholdImageFilter<FImg, FImg>, itk::DoubleThresholdImageFilter<DImg, DImg>>(
    "MathematicalMorphology", "DoubleThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetThreshold1(20); f->SetThreshold2(40); f->SetThreshold3(180); f->SetThreshold4(220); },
    [&](auto * f) { f->SetThreshold1(20); f->SetThreshold2(40); f->SetThreshold3(180); f->SetThreshold4(220); });

  BenchTime(
    "ImageGradient",
    "GradientRecursiveGaussianImageFilter",
    [&]() {
      auto f = itk::GradientRecursiveGaussianImageFilter<FImg>::New();
      f->SetInput(input);
      f->SetSigma(1.5);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::GradientRecursiveGaussianImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->SetSigma(1.5);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGradient",
    "DifferenceOfGaussiansGradientImageFilter",
    [&]() {
      auto f = itk::DifferenceOfGaussiansGradientImageFilter<FImg, float>::New();
      f->SetInput(input);
      f->SetWidth(1.2);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::DifferenceOfGaussiansGradientImageFilter<DImg, double>::New();
      f->SetInput(c->GetOutput());
      f->SetWidth(1.2);
      f->Update();
    },
    runs);

  BenchTime(
    "ImageFeature",
    "HessianRecursiveGaussianImageFilter",
    [&]() {
      auto f = itk::HessianRecursiveGaussianImageFilter<FImg>::New();
      f->SetInput(input);
      f->SetSigma(1.5);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::HessianRecursiveGaussianImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->SetSigma(1.5);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFeature",
    "HoughTransform2DCirclesImageFilter",
    [&]() {
      auto f = itk::HoughTransform2DCirclesImageFilter<float, float, float>::New();
      f->SetInput(input);
      f->SetNumberOfCircles(2);
      f->SetMinimumRadius(4.0);
      f->SetMaximumRadius(20.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::HoughTransform2DCirclesImageFilter<double, double, double>::New();
      f->SetInput(c->GetOutput());
      f->SetNumberOfCircles(2);
      f->SetMinimumRadius(4.0);
      f->SetMaximumRadius(20.0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFeature",
    "HoughTransform2DLinesImageFilter",
    [&]() {
      auto f = itk::HoughTransform2DLinesImageFilter<float, float>::New();
      f->SetInput(input);
      f->SetNumberOfLines(2);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::HoughTransform2DLinesImageFilter<double, double>::New();
      f->SetInput(c->GetOutput());
      f->SetNumberOfLines(2);
      f->Update();
    },
    runs);

  Run2<itk::FFTDiscreteGaussianImageFilter<FImg, FImg>, itk::FFTDiscreteGaussianImageFilter<DImg, DImg>>(
    "Smoothing", "FFTDiscreteGaussianImageFilter", input, runs,
    [&](auto * f) { f->SetVariance(4.0); }, [&](auto * f) { f->SetVariance(4.0); });

  Bench(
    "ImageGrid",
    "TileImageFilter",
    [&]() {
      auto f = itk::TileImageFilter<FImg, FImg>::New();
      itk::FixedArray<unsigned int, 2> layout;
      layout[0] = 2;
      layout[1] = 1;
      f->SetLayout(layout);
      f->SetInput(0, input);
      f->SetInput(1, input);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::TileImageFilter<DImg, DImg>::New();
      itk::FixedArray<unsigned int, 2> layout;
      layout[0] = 2;
      layout[1] = 1;
      f->SetLayout(layout);
      f->SetInput(0, c->GetOutput());
      f->SetInput(1, c->GetOutput());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Run2<itk::BSplineDownsampleImageFilter<FImg, FImg>, itk::BSplineDownsampleImageFilter<DImg, DImg>>(
    "ImageGrid", "BSplineDownsampleImageFilter", input, runs, [&](auto *) {}, [&](auto *) {});
  Run2<itk::BSplineUpsampleImageFilter<FImg, FImg>, itk::BSplineUpsampleImageFilter<DImg, DImg>>(
    "ImageGrid", "BSplineUpsampleImageFilter", input, runs, [&](auto *) {}, [&](auto *) {});

  Bench(
    "Deconvolution",
    "InverseDeconvolutionImageFilter",
    [&]() {
      using F = itk::InverseDeconvolutionImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetKernelImage(kernelF3);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::InverseDeconvolutionImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetKernelImage(kernelD3);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Deconvolution",
    "ProjectedLandweberDeconvolutionImageFilter",
    [&]() {
      using F = itk::ProjectedLandweberDeconvolutionImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetKernelImage(kernelF3);
      f->SetNumberOfIterations(3);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::ProjectedLandweberDeconvolutionImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetKernelImage(kernelD3);
      f->SetNumberOfIterations(3);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  BenchTime(
    "DisplacementField",
    "ComposeDisplacementFieldsImageFilter",
    [&]() {
      auto f = itk::ComposeDisplacementFieldsImageFilter<VFImg>::New();
      f->SetDisplacementField(dispF2);
      f->SetWarpingField(dispF2);
      f->Update();
    },
    [&]() {
      auto f = itk::ComposeDisplacementFieldsImageFilter<VDImg>::New();
      f->SetDisplacementField(dispD2);
      f->SetWarpingField(dispD2);
      f->Update();
    },
    runs);
  BenchTime(
    "DisplacementField",
    "ExponentialDisplacementFieldImageFilter",
    [&]() {
      auto f = itk::ExponentialDisplacementFieldImageFilter<VFImg, VFImg>::New();
      f->SetInput(dispF2);
      f->SetMaximumNumberOfIterations(4);
      f->Update();
    },
    [&]() {
      auto f = itk::ExponentialDisplacementFieldImageFilter<VDImg, VDImg>::New();
      f->SetInput(dispD2);
      f->SetMaximumNumberOfIterations(4);
      f->Update();
    },
    runs);
  BenchTime(
    "DisplacementField",
    "InverseDisplacementFieldImageFilter",
    [&]() {
      auto f = itk::InverseDisplacementFieldImageFilter<VFImg, VFImg>::New();
      f->SetInput(dispF2);
      f->SetSize(input->GetLargestPossibleRegion().GetSize());
      f->Update();
    },
    [&]() {
      auto f = itk::InverseDisplacementFieldImageFilter<VDImg, VDImg>::New();
      f->SetInput(dispD2);
      f->SetSize(input->GetLargestPossibleRegion().GetSize());
      f->Update();
    },
    runs);

  BenchTime(
    "DistanceMap",
    "ContourMeanDistanceImageFilter",
    [&]() {
      auto f = itk::ContourMeanDistanceImageFilter<FImg, FImg>::New();
      f->SetInput1(binF);
      f->SetInput2(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::ContourMeanDistanceImageFilter<DImg, DImg>::New();
      f->SetInput1(binD);
      f->SetInput2(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "DistanceMap",
    "DirectedHausdorffDistanceImageFilter",
    [&]() {
      auto f = itk::DirectedHausdorffDistanceImageFilter<FImg, FImg>::New();
      f->SetInput1(binF);
      f->SetInput2(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::DirectedHausdorffDistanceImageFilter<DImg, DImg>::New();
      f->SetInput1(binD);
      f->SetInput2(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "DistanceMap",
    "ContourDirectedMeanDistanceImageFilter",
    [&]() {
      auto f = itk::ContourDirectedMeanDistanceImageFilter<FImg, FImg>::New();
      f->SetInput1(binF);
      f->SetInput2(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::ContourDirectedMeanDistanceImageFilter<DImg, DImg>::New();
      f->SetInput1(binD);
      f->SetInput2(binD);
      f->Update();
    },
    runs);

  BenchTime(
    "FFT",
    "VnlForwardFFTImageFilter",
    [&]() {
      auto f = itk::VnlForwardFFTImageFilter<FImg, CImgF>::New();
      f->SetInput(input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::VnlForwardFFTImageFilter<DImg, CImgD>::New();
      f->SetInput(c->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "VnlInverseFFTImageFilter",
    [&]() {
      auto fwd = itk::VnlForwardFFTImageFilter<FImg, CImgF>::New();
      fwd->SetInput(input);
      auto f = itk::VnlInverseFFTImageFilter<CImgF, FImg>::New();
      f->SetInput(fwd->GetOutput());
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto fwd = itk::VnlForwardFFTImageFilter<DImg, CImgD>::New();
      fwd->SetInput(c->GetOutput());
      auto f = itk::VnlInverseFFTImageFilter<CImgD, DImg>::New();
      f->SetInput(fwd->GetOutput());
      f->Update();
    },
    runs);

  BenchTime(
    "ImageSources",
    "GaussianImageSource",
    [&]() {
      auto s = itk::GaussianImageSource<FImg>::New();
      s->SetSize(input->GetLargestPossibleRegion().GetSize());
      s->SetSpacing(input->GetSpacing());
      s->SetOrigin(input->GetOrigin());
      s->Update();
    },
    [&]() {
      auto s = itk::GaussianImageSource<DImg>::New();
      s->SetSize(input->GetLargestPossibleRegion().GetSize());
      s->SetSpacing(input->GetSpacing());
      s->SetOrigin(input->GetOrigin());
      s->Update();
    },
    runs);
  BenchTime(
    "ImageSources",
    "GaborImageSource",
    [&]() {
      auto s = itk::GaborImageSource<FImg>::New();
      s->SetSize(input->GetLargestPossibleRegion().GetSize());
      s->SetSpacing(input->GetSpacing());
      s->SetOrigin(input->GetOrigin());
      s->Update();
    },
    [&]() {
      auto s = itk::GaborImageSource<DImg>::New();
      s->SetSize(input->GetLargestPossibleRegion().GetSize());
      s->SetSpacing(input->GetSpacing());
      s->SetOrigin(input->GetOrigin());
      s->Update();
    },
    runs);

  BenchTime(
    "ImageStatistics",
    "StatisticsImageFilter",
    [&]() {
      auto f = itk::StatisticsImageFilter<FImg>::New();
      f->SetInput(input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::StatisticsImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "MinimumMaximumImageFilter",
    [&]() {
      auto f = itk::MinimumMaximumImageFilter<FImg>::New();
      f->SetInput(input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::MinimumMaximumImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "AccumulateImageFilter",
    [&]() {
      auto f = itk::AccumulateImageFilter<FImg, FImg>::New();
      f->SetInput(input);
      f->SetAccumulateDimension(1);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::AccumulateImageFilter<DImg, DImg>::New();
      f->SetInput(c->GetOutput());
      f->SetAccumulateDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "MedianProjectionImageFilter",
    [&]() {
      auto f = itk::MedianProjectionImageFilter<FImg, itk::Image<float, 1>>::New();
      f->SetInput(input);
      f->SetProjectionDimension(1);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::MedianProjectionImageFilter<DImg, itk::Image<double, 1>>::New();
      f->SetInput(c->GetOutput());
      f->SetProjectionDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "StandardDeviationProjectionImageFilter",
    [&]() {
      auto f = itk::StandardDeviationProjectionImageFilter<FImg, itk::Image<float, 1>>::New();
      f->SetInput(input);
      f->SetProjectionDimension(1);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::StandardDeviationProjectionImageFilter<DImg, itk::Image<double, 1>>::New();
      f->SetInput(c->GetOutput());
      f->SetProjectionDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "BinaryProjectionImageFilter",
    [&]() {
      auto f = itk::BinaryProjectionImageFilter<FImg, itk::Image<float, 1>>::New();
      f->SetInput(binF);
      f->SetProjectionDimension(1);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryProjectionImageFilter<DImg, itk::Image<double, 1>>::New();
      f->SetInput(binD);
      f->SetProjectionDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "Thresholding",
    "BinaryThresholdProjectionImageFilter",
    [&]() {
      auto f = itk::BinaryThresholdProjectionImageFilter<FImg, itk::Image<unsigned char, 1>>::New();
      f->SetInput(input);
      f->SetProjectionDimension(1);
      f->SetThresholdValue(100.0f);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::BinaryThresholdProjectionImageFilter<DImg, itk::Image<unsigned char, 1>>::New();
      f->SetInput(c->GetOutput());
      f->SetProjectionDimension(1);
      f->SetThresholdValue(100.0);
      f->Update();
    },
    runs);

  RunBinMorph<itk::FastIncrementalBinaryDilateImageFilter<FImg, FImg, BallF>, itk::FastIncrementalBinaryDilateImageFilter<DImg, DImg, BallD>>(
    "BinaryMathematicalMorphology", "FastIncrementalBinaryDilateImageFilter", binF, binD, kf, kd, runs);

  BenchTime(
    "ConnectedComponents",
    "HardConnectedComponentImageFilter",
    [&]() {
      auto f = itk::HardConnectedComponentImageFilter<FImg, U16Img>::New();
      f->SetInput(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::HardConnectedComponentImageFilter<DImg, U16Img>::New();
      f->SetInput(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "ConnectedComponents",
    "ThresholdMaximumConnectedComponentsImageFilter",
    [&]() {
      auto f = itk::ThresholdMaximumConnectedComponentsImageFilter<FImg>::New();
      f->SetInput(input);
      f->SetInsideValue(1);
      f->SetOutsideValue(0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::ThresholdMaximumConnectedComponentsImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->SetInsideValue(1);
      f->SetOutsideValue(0);
      f->Update();
    },
    runs);

  BenchTime(
    "LabelMap",
    "BinaryShapeKeepNObjectsImageFilter",
    [&]() {
      auto f = itk::BinaryShapeKeepNObjectsImageFilter<FImg>::New();
      f->SetInput(binF);
      f->SetForegroundValue(1.0f);
      f->SetNumberOfObjects(5);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryShapeKeepNObjectsImageFilter<DImg>::New();
      f->SetInput(binD);
      f->SetForegroundValue(1.0);
      f->SetNumberOfObjects(5);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ShapeRelabelImageFilter",
    [&]() {
      auto cc = itk::ConnectedComponentImageFilter<FImg, U16Img>::New();
      cc->SetInput(binF);
      auto f = itk::ShapeRelabelImageFilter<U16Img>::New();
      f->SetInput(cc->GetOutput());
      f->Update();
    },
    [&]() {
      auto cc = itk::ConnectedComponentImageFilter<DImg, U16Img>::New();
      cc->SetInput(binD);
      auto f = itk::ShapeRelabelImageFilter<U16Img>::New();
      f->SetInput(cc->GetOutput());
      f->Update();
    },
    runs);

  BenchTime(
    "LabelVoting",
    "LabelVotingImageFilter",
    [&]() {
      auto cc = itk::ConnectedComponentImageFilter<FImg, U16Img>::New();
      cc->SetInput(binF);
      cc->Update();
      auto f = itk::LabelVotingImageFilter<U16Img, U16Img>::New();
      f->SetInput(0, cc->GetOutput());
      f->SetInput(1, cc->GetOutput());
      f->Update();
    },
    [&]() {
      auto cc = itk::ConnectedComponentImageFilter<DImg, U16Img>::New();
      cc->SetInput(binD);
      cc->Update();
      auto f = itk::LabelVotingImageFilter<U16Img, U16Img>::New();
      f->SetInput(0, cc->GetOutput());
      f->SetInput(1, cc->GetOutput());
      f->Update();
    },
    runs);
  Bench(
    "ImageCompare",
    "STAPLEImageFilter",
    [&]() {
      auto f = itk::STAPLEImageFilter<FImg, FImg>::New();
      f->SetInput(0, binF);
      f->SetInput(1, binF);
      f->SetForegroundValue(1.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      auto f = itk::STAPLEImageFilter<DImg, DImg>::New();
      f->SetInput(0, binD);
      f->SetInput(1, binD);
      f->SetForegroundValue(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  BenchTime(
    "Watersheds",
    "WatershedImageFilter",
    [&]() {
      auto f = itk::WatershedImageFilter<FImg>::New();
      f->SetInput(input);
      f->SetLevel(0.2);
      f->SetThreshold(0.05);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::WatershedImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->SetLevel(0.2);
      f->SetThreshold(0.05);
      f->Update();
    },
    runs);
  BenchTime(
    "Watersheds",
    "IsolatedWatershedImageFilter",
    [&]() {
      auto f = itk::IsolatedWatershedImageFilter<FImg, FImg>::New();
      f->SetInput(input);
      f->SetSeed1(seed);
      FImg::IndexType s2 = seed;
      s2[0] += 20;
      f->SetSeed2(s2);
      f->SetThreshold(0.001);
      f->SetUpperValueLimit(1.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::IsolatedWatershedImageFilter<DImg, DImg>::New();
      f->SetInput(c->GetOutput());
      f->SetSeed1(seed);
      DImg::IndexType s2 = seed;
      s2[0] += 20;
      f->SetSeed2(s2);
      f->SetThreshold(0.001);
      f->SetUpperValueLimit(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "Watersheds",
    "MorphologicalWatershedFromMarkersImageFilter",
    [&]() {
      auto f = itk::MorphologicalWatershedFromMarkersImageFilter<FImg, U16Img>::New();
      f->SetInput(input);
      f->SetMarkerImage(lab16b);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::MorphologicalWatershedFromMarkersImageFilter<DImg, U16Img>::New();
      f->SetInput(c->GetOutput());
      f->SetMarkerImage(lab16b);
      f->Update();
    },
    runs);

  BenchTime(
    "PDEDeformable",
    "SymmetricForcesDemonsRegistrationFilter",
    [&]() {
      auto f = itk::SymmetricForcesDemonsRegistrationFilter<FImg, FImg, VFImg>::New();
      f->SetFixedImage(input);
      f->SetMovingImage(input);
      f->SetNumberOfIterations(4);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::SymmetricForcesDemonsRegistrationFilter<DImg, DImg, VDImg>::New();
      f->SetFixedImage(c->GetOutput());
      f->SetMovingImage(c->GetOutput());
      f->SetNumberOfIterations(4);
      f->Update();
    },
    runs);
  BenchTime(
    "PDEDeformable",
    "FastSymmetricForcesDemonsRegistrationFilter",
    [&]() {
      auto f = itk::FastSymmetricForcesDemonsRegistrationFilter<FImg, FImg, VFImg>::New();
      f->SetFixedImage(input);
      f->SetMovingImage(input);
      f->SetNumberOfIterations(4);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::FastSymmetricForcesDemonsRegistrationFilter<DImg, DImg, VDImg>::New();
      f->SetFixedImage(c->GetOutput());
      f->SetMovingImage(c->GetOutput());
      f->SetNumberOfIterations(4);
      f->Update();
    },
    runs);
}
