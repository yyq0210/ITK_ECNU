// 造数据补测：二值/整数/标签/向量/两图/核/投影。串行算子不要写在这里。
{
  using U8Img = itk::Image<unsigned char, Dim>;
  using U16Img = itk::Image<unsigned short, Dim>;
  using Vec2F = itk::Vector<float, 2>;
  using Vec2D = itk::Vector<double, 2>;
  using VFImg = itk::Image<Vec2F, Dim>;
  using VDImg = itk::Image<Vec2D, Dim>;
  using LO = itk::LabelObject<unsigned long, Dim>;
  using LM = itk::LabelMap<LO>;
  using SLO = itk::ShapeLabelObject<unsigned long, Dim>;
  using SLM = itk::LabelMap<SLO>;
  using RGBImg = itk::Image<itk::RGBPixel<unsigned char>, Dim>;

  U8Img::Pointer  u8;
  U8Img::Pointer  u8b;
  FImg::Pointer   kernelF;
  DImg::Pointer   kernelD;
  VFImg::Pointer  dispF;
  VDImg::Pointer  dispD;
  U16Img::Pointer lab16;

  if (!g_list)
  {
    using RU = itk::RescaleIntensityImageFilter<FImg, U8Img>;
    auto ru = RU::New();
    ru->SetInput(input);
    ru->SetOutputMinimum(0);
    ru->SetOutputMaximum(255);
    ru->Update();
    u8 = ru->GetOutput();
    u8->DisconnectPipeline();

    using RUb = itk::RescaleIntensityImageFilter<FImg, U8Img>;
    auto rb = RUb::New();
    rb->SetInput(binF);
    rb->SetOutputMinimum(0);
    rb->SetOutputMaximum(1);
    rb->Update();
    u8b = rb->GetOutput();
    u8b->DisconnectPipeline();

    using Gsrc = itk::GaussianImageSource<FImg>;
    auto gs = Gsrc::New();
    FImg::SizeType ksz;
    ksz.Fill(9);
    gs->SetSize(ksz);
    gs->SetNormalized(true);
    gs->SetSigma(1.5);
    gs->Update();
    kernelF = gs->GetOutput();
    kernelF->DisconnectPipeline();
    using CK = itk::CastImageFilter<FImg, DImg>;
    auto ck = CK::New();
    ck->SetInput(kernelF);
    ck->Update();
    kernelD = ck->GetOutput();
    kernelD->DisconnectPipeline();

    dispF = VFImg::New();
    dispF->CopyInformation(input);
    dispF->SetRegions(input->GetLargestPossibleRegion());
    dispF->Allocate();
    Vec2F zf;
    zf[0] = 0.4f;
    zf[1] = -0.3f;
    dispF->FillBuffer(zf);
    dispD = VDImg::New();
    dispD->CopyInformation(input);
    dispD->SetRegions(input->GetLargestPossibleRegion());
    dispD->Allocate();
    Vec2D zd;
    zd[0] = 0.4;
    zd[1] = -0.3;
    dispD->FillBuffer(zd);

    auto cc = itk::ConnectedComponentImageFilter<FImg, U16Img>::New();
    cc->SetInput(binF);
    cc->Update();
    lab16 = cc->GetOutput();
    lab16->DisconnectPipeline();
  }

  Run1<itk::ThresholdImageFilter<FImg>, itk::ThresholdImageFilter<DImg>>(
    "Thresholding", "ThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetLower(20.0f); f->SetUpper(220.0f); },
    [&](auto * f) { f->SetLower(20.0); f->SetUpper(220.0); });
  Run2<itk::HistogramThresholdImageFilter<FImg, FImg>, itk::HistogramThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "HistogramThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::KappaSigmaThresholdImageFilter<FImg, FImg>, itk::KappaSigmaThresholdImageFilter<DImg, DImg>>(
    "Thresholding", "KappaSigmaThresholdImageFilter", input, runs,
    [&](auto * f) { f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f); },
    [&](auto * f) { f->SetInsideValue(1.0); f->SetOutsideValue(0.0); });
  Run2<itk::ThresholdLabelerImageFilter<FImg, FImg>, itk::ThresholdLabelerImageFilter<DImg, DImg>>(
    "Thresholding", "ThresholdLabelerImageFilter", input, runs, [&](auto *) {}, [&](auto *) {});

  Run1<itk::RecursiveGaussianImageFilter<FImg>, itk::RecursiveGaussianImageFilter<DImg>>(
    "Smoothing", "RecursiveGaussianImageFilter", input, runs,
    [&](auto * f) { f->SetSigma(2.0); f->SetDirection(0); },
    [&](auto * f) { f->SetSigma(2.0); f->SetDirection(0); });
  Run2<itk::NoiseImageFilter<FImg, FImg>, itk::NoiseImageFilter<DImg, DImg>>(
    "ImageFilterBase", "NoiseImageFilter", input, runs,
    [&](auto * f) { f->SetRadius(radius); }, [&](auto * f) { f->SetRadius(radius); });

  Run2<itk::MaskImageFilter<FImg, FImg, FImg>, itk::MaskImageFilter<DImg, DImg, DImg>>(
    "ImageIntensity", "MaskImageFilter", input, runs,
    [&](auto * f) { f->SetMaskImage(binF); }, [&](auto * f) { f->SetMaskImage(binD); });
  Run2<itk::MaskNegatedImageFilter<FImg, FImg, FImg>, itk::MaskNegatedImageFilter<DImg, DImg, DImg>>(
    "ImageIntensity", "MaskNegatedImageFilter", input, runs,
    [&](auto * f) { f->SetMaskImage(binF); }, [&](auto * f) { f->SetMaskImage(binD); });
  Run2<itk::HistogramMatchingImageFilter<FImg, FImg>, itk::HistogramMatchingImageFilter<DImg, DImg>>(
    "ImageIntensity", "HistogramMatchingImageFilter", input, runs,
    [&](auto * f) { f->SetReferenceImage(input); f->SetNumberOfHistogramLevels(64); f->SetNumberOfMatchPoints(8); },
    [&](auto * f) {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      f->SetReferenceImage(c->GetOutput());
      f->SetNumberOfHistogramLevels(64);
      f->SetNumberOfMatchPoints(8);
    });
  BINARY_SELF("ImageIntensity", ConstrainedValueAdditionImageFilter);
  BINARY_SELF("ImageIntensity", WeightedAddImageFilter);
  Bench(
    "ImageCompare",
    "CheckerBoardImageFilter",
    [&]() {
      using F = itk::CheckerBoardImageFilter<FImg>;
      auto f = F::New();
      f->SetInput1(input);
      f->SetInput2(input);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::CheckerBoardImageFilter<DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = F::New();
      f->SetInput1(c->GetOutput());
      f->SetInput2(c->GetOutput());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  RunBin<itk::BinaryMinMaxCurvatureFlowImageFilter<FImg, FImg>, itk::BinaryMinMaxCurvatureFlowImageFilter<DImg, DImg>>(
    "CurvatureFlow", "BinaryMinMaxCurvatureFlowImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetTimeStep(0.125); f->SetNumberOfIterations(4); f->SetStencilRadius(1); },
    [&](auto * f) { f->SetTimeStep(0.125); f->SetNumberOfIterations(4); f->SetStencilRadius(1); });

  RunBin<itk::DilateObjectMorphologyImageFilter<FImg, FImg, BallF>, itk::DilateObjectMorphologyImageFilter<DImg, DImg, BallD>>(
    "BinaryMathematicalMorphology", "DilateObjectMorphologyImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetKernel(kf); f->SetObjectValue(1.0f); },
    [&](auto * f) { f->SetKernel(kd); f->SetObjectValue(1.0); });
  RunBin<itk::ErodeObjectMorphologyImageFilter<FImg, FImg, BallF>, itk::ErodeObjectMorphologyImageFilter<DImg, DImg, BallD>>(
    "BinaryMathematicalMorphology", "ErodeObjectMorphologyImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetKernel(kf); f->SetObjectValue(1.0); f->SetBackgroundValue(0.0); },
    [&](auto * f) { f->SetKernel(kd); f->SetObjectValue(1.0); f->SetBackgroundValue(0.0); });
  RunBin<itk::VotingBinaryIterativeHoleFillingImageFilter<FImg>, itk::VotingBinaryIterativeHoleFillingImageFilter<DImg>>(
    "LabelVoting", "VotingBinaryIterativeHoleFillingImageFilter", binF, binD, runs,
    [&](auto * f) { f->SetRadius(radius); f->SetMaximumNumberOfIterations(2); f->SetForegroundValue(1.0f); f->SetBackgroundValue(0.0f); },
    [&](auto * f) { f->SetRadius(radius); f->SetMaximumNumberOfIterations(2); f->SetForegroundValue(1.0); f->SetBackgroundValue(0.0); });

  Bench(
    "Convolution",
    "FFTNormalizedCorrelationImageFilter",
    [&]() {
      using F = itk::FFTNormalizedCorrelationImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetFixedImage(input);
      f->SetMovingImage(input);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::FFTNormalizedCorrelationImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = F::New();
      f->SetFixedImage(c->GetOutput());
      f->SetMovingImage(c->GetOutput());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Convolution",
    "NormalizedCorrelationImageFilter",
    [&]() {
      using F = itk::NormalizedCorrelationImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetMaskImage(binF);
      itk::Neighborhood<float, 2> t;
      t.SetRadius(1);
      for (unsigned int i = 0; i < t.Size(); ++i)
      {
        t[i] = 1.0f;
      }
      f->SetTemplate(t);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::NormalizedCorrelationImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetMaskImage(binD);
      itk::Neighborhood<double, 2> t;
      t.SetRadius(1);
      for (unsigned int i = 0; i < t.Size(); ++i)
      {
        t[i] = 1.0;
      }
      f->SetTemplate(t);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Convolution",
    "MaskedFFTNormalizedCorrelationImageFilter",
    [&]() {
      using F = itk::MaskedFFTNormalizedCorrelationImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetFixedImage(input);
      f->SetMovingImage(input);
      f->SetFixedImageMask(binF);
      f->SetMovingImageMask(binF);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::MaskedFFTNormalizedCorrelationImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = F::New();
      f->SetFixedImage(c->GetOutput());
      f->SetMovingImage(c->GetOutput());
      f->SetFixedImageMask(binD);
      f->SetMovingImageMask(binD);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  Run1<itk::PermuteAxesImageFilter<FImg>, itk::PermuteAxesImageFilter<DImg>>(
    "ImageGrid", "PermuteAxesImageFilter", input, runs,
    [&](auto * f) {
      typename itk::PermuteAxesImageFilter<FImg>::PermuteOrderArrayType o;
      o[0] = 1;
      o[1] = 0;
      f->SetOrder(o);
    },
    [&](auto * f) {
      typename itk::PermuteAxesImageFilter<DImg>::PermuteOrderArrayType o;
      o[0] = 1;
      o[1] = 0;
      f->SetOrder(o);
    });
  Bench(
    "ImageGrid",
    "PasteImageFilter",
    [&]() {
      using F = itk::PasteImageFilter<FImg>;
      auto f = F::New();
      f->SetDestinationImage(input);
      f->SetSourceImage(input);
      f->SetSourceRegion(roi);
      f->SetDestinationIndex(input->GetLargestPossibleRegion().GetIndex());
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::PasteImageFilter<DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = F::New();
      f->SetDestinationImage(c->GetOutput());
      f->SetSourceImage(c->GetOutput());
      DImg::RegionType r = c->GetOutput()->GetLargestPossibleRegion();
      DImg::IndexType  idx = r.GetIndex();
      DImg::SizeType   sz = r.GetSize();
      const long       crop = 16;
      idx[0] += crop;
      idx[1] += crop;
      sz[0] -= static_cast<unsigned long>(2 * crop);
      sz[1] -= static_cast<unsigned long>(2 * crop);
      r.SetIndex(idx);
      r.SetSize(sz);
      f->SetSourceRegion(r);
      f->SetDestinationIndex(c->GetOutput()->GetLargestPossibleRegion().GetIndex());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  BenchTime(
    "ImageGrid",
    "ExtractImageFilter",
    [&]() {
      using F = itk::ExtractImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetExtractionRegion(roi);
      f->SetDirectionCollapseToIdentity();
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::ExtractImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      DImg::RegionType r;
      DImg::IndexType  idx = input->GetLargestPossibleRegion().GetIndex();
      DImg::SizeType   sz = input->GetLargestPossibleRegion().GetSize();
      const long       crop = 16;
      idx[0] += crop;
      idx[1] += crop;
      sz[0] -= static_cast<unsigned long>(2 * crop);
      sz[1] -= static_cast<unsigned long>(2 * crop);
      r.SetIndex(idx);
      r.SetSize(sz);
      f->SetExtractionRegion(r);
      f->SetDirectionCollapseToIdentity();
      f->Update();
    },
    runs);
  BenchTime(
    "ImageCompose",
    "JoinSeriesImageFilter",
    [&]() {
      using F = itk::JoinSeriesImageFilter<FImg, itk::Image<float, 3>>;
      auto f = F::New();
      f->SetInput(0, input);
      f->SetInput(1, input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::JoinSeriesImageFilter<DImg, itk::Image<double, 3>>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = F::New();
      f->SetInput(0, c->GetOutput());
      f->SetInput(1, c->GetOutput());
      f->Update();
    },
    runs);

  BenchTime(
    "ImageStatistics",
    "MeanProjectionImageFilter",
    [&]() {
      auto f = itk::MeanProjectionImageFilter<FImg, itk::Image<float, 1>>::New();
      f->SetInput(input);
      f->SetProjectionDimension(1);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::MeanProjectionImageFilter<DImg, itk::Image<double, 1>>::New();
      f->SetInput(c->GetOutput());
      f->SetProjectionDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "MaximumProjectionImageFilter",
    [&]() {
      auto f = itk::MaximumProjectionImageFilter<FImg, itk::Image<float, 1>>::New();
      f->SetInput(input);
      f->SetProjectionDimension(1);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::MaximumProjectionImageFilter<DImg, itk::Image<double, 1>>::New();
      f->SetInput(c->GetOutput());
      f->SetProjectionDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "MinimumProjectionImageFilter",
    [&]() {
      auto f = itk::MinimumProjectionImageFilter<FImg, itk::Image<float, 1>>::New();
      f->SetInput(input);
      f->SetProjectionDimension(1);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::MinimumProjectionImageFilter<DImg, itk::Image<double, 1>>::New();
      f->SetInput(c->GetOutput());
      f->SetProjectionDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "SumProjectionImageFilter",
    [&]() {
      auto f = itk::SumProjectionImageFilter<FImg, itk::Image<float, 1>>::New();
      f->SetInput(input);
      f->SetProjectionDimension(1);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::SumProjectionImageFilter<DImg, itk::Image<double, 1>>::New();
      f->SetInput(c->GetOutput());
      f->SetProjectionDimension(1);
      f->Update();
    },
    runs);

  Bench(
    "Deconvolution",
    "WienerDeconvolutionImageFilter",
    [&]() {
      using F = itk::WienerDeconvolutionImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetKernelImage(kernelF);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::WienerDeconvolutionImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetKernelImage(kernelD);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Deconvolution",
    "TikhonovDeconvolutionImageFilter",
    [&]() {
      using F = itk::TikhonovDeconvolutionImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetKernelImage(kernelF);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::TikhonovDeconvolutionImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetKernelImage(kernelD);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Deconvolution",
    "LandweberDeconvolutionImageFilter",
    [&]() {
      using F = itk::LandweberDeconvolutionImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetKernelImage(kernelF);
      f->SetNumberOfIterations(3);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::LandweberDeconvolutionImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetKernelImage(kernelD);
      f->SetNumberOfIterations(3);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Deconvolution",
    "RichardsonLucyDeconvolutionImageFilter",
    [&]() {
      using F = itk::RichardsonLucyDeconvolutionImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetKernelImage(kernelF);
      f->SetNumberOfIterations(3);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::RichardsonLucyDeconvolutionImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetKernelImage(kernelD);
      f->SetNumberOfIterations(3);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  Bench(
    "BiasCorrection",
    "N4BiasFieldCorrectionImageFilter",
    [&]() {
      using F = itk::N4BiasFieldCorrectionImageFilter<FImg, U8Img, FImg>;
      auto f = F::New();
      f->SetInput(input);
      itk::Array<unsigned int> n4it(2);
      n4it.Fill(2);
      f->SetMaximumNumberOfIterations(n4it);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::N4BiasFieldCorrectionImageFilter<DImg, U8Img, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      itk::Array<unsigned int> n4it(2);
      n4it.Fill(2);
      f->SetMaximumNumberOfIterations(n4it);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  BenchTime(
    "Classifiers",
    "ScalarImageKmeansImageFilter",
    [&]() {
      auto f = itk::ScalarImageKmeansImageFilter<FImg>::New();
      f->SetInput(input);
      f->AddClassWithInitialMean(50.0);
      f->AddClassWithInitialMean(150.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::ScalarImageKmeansImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->AddClassWithInitialMean(50.0);
      f->AddClassWithInitialMean(150.0);
      f->Update();
    },
    runs);
  BenchTime(
    "SuperPixel",
    "SLICImageFilter",
    [&]() {
      auto f = itk::SLICImageFilter<FImg, U16Img>::New();
      f->SetInput(input);
      f->SetSuperGridSize(32);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::SLICImageFilter<DImg, U16Img>::New();
      f->SetInput(c->GetOutput());
      f->SetSuperGridSize(32);
      f->Update();
    },
    runs);
  BenchTime(
    "Watersheds",
    "MorphologicalWatershedImageFilter",
    [&]() {
      auto f = itk::MorphologicalWatershedImageFilter<FImg, U16Img>::New();
      f->SetInput(input);
      f->SetLevel(2.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::MorphologicalWatershedImageFilter<DImg, U16Img>::New();
      f->SetInput(c->GetOutput());
      f->SetLevel(2.0);
      f->Update();
    },
    runs);
  BenchTime(
    "Watersheds",
    "TobogganImageFilter",
    [&]() {
      auto f = itk::TobogganImageFilter<FImg>::New();
      f->SetInput(input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::TobogganImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->Update();
    },
    runs);

  BenchTime(
    "ImageIntensity",
    "AndImageFilter",
    [&]() {
      auto f = itk::AndImageFilter<U8Img, U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    [&]() {
      auto f = itk::AndImageFilter<U8Img, U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "OrImageFilter",
    [&]() {
      auto f = itk::OrImageFilter<U8Img, U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    [&]() {
      auto f = itk::OrImageFilter<U8Img, U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "XorImageFilter",
    [&]() {
      auto f = itk::XorImageFilter<U8Img, U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    [&]() {
      auto f = itk::XorImageFilter<U8Img, U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "NotImageFilter",
    [&]() {
      auto f = itk::NotImageFilter<U8Img, U8Img>::New();
      f->SetInput(u8);
      f->Update();
    },
    [&]() {
      auto f = itk::NotImageFilter<U8Img, U8Img>::New();
      f->SetInput(u8);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "BinaryNotImageFilter",
    [&]() {
      auto f = itk::BinaryNotImageFilter<U8Img>::New();
      f->SetInput(u8b);
      f->SetForegroundValue(1);
      f->SetBackgroundValue(0);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryNotImageFilter<U8Img>::New();
      f->SetInput(u8b);
      f->SetForegroundValue(1);
      f->SetBackgroundValue(0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "ModulusImageFilter",
    [&]() {
      auto f = itk::ModulusImageFilter<U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    [&]() {
      auto f = itk::ModulusImageFilter<U8Img, U8Img>::New();
      f->SetInput1(u8);
      f->SetInput2(u8);
      f->Update();
    },
    runs);

  BenchTime(
    "ConnectedComponents",
    "ConnectedComponentImageFilter",
    [&]() {
      auto f = itk::ConnectedComponentImageFilter<FImg, U16Img>::New();
      f->SetInput(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::ConnectedComponentImageFilter<DImg, U16Img>::New();
      f->SetInput(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "ConnectedComponents",
    "RelabelComponentImageFilter",
    [&]() {
      auto f = itk::RelabelComponentImageFilter<U16Img, U16Img>::New();
      f->SetInput(lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::RelabelComponentImageFilter<U16Img, U16Img>::New();
      f->SetInput(lab16);
      f->Update();
    },
    runs);
  BenchTime(
    "ConnectedComponents",
    "ScalarConnectedComponentImageFilter",
    [&]() {
      auto f = itk::ScalarConnectedComponentImageFilter<FImg, U16Img>::New();
      f->SetInput(input);
      f->SetDistanceThreshold(8.0f);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::ScalarConnectedComponentImageFilter<DImg, U16Img>::New();
      f->SetInput(c->GetOutput());
      f->SetDistanceThreshold(8.0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageLabel",
    "LabelContourImageFilter",
    [&]() {
      auto f = itk::LabelContourImageFilter<U16Img, U16Img>::New();
      f->SetInput(lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelContourImageFilter<U16Img, U16Img>::New();
      f->SetInput(lab16);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageLabel",
    "ChangeLabelImageFilter",
    [&]() {
      auto f = itk::ChangeLabelImageFilter<U16Img, U16Img>::New();
      f->SetInput(lab16);
      f->SetChange(1, 2);
      f->Update();
    },
    [&]() {
      auto f = itk::ChangeLabelImageFilter<U16Img, U16Img>::New();
      f->SetInput(lab16);
      f->SetChange(1, 2);
      f->Update();
    },
    runs);

  BenchTime(
    "LabelMap",
    "BinaryImageToLabelMapFilter",
    [&]() {
      auto f = itk::BinaryImageToLabelMapFilter<FImg, LM>::New();
      f->SetInput(binF);
      f->SetInputForegroundValue(1.0f);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryImageToLabelMapFilter<DImg, LM>::New();
      f->SetInput(binD);
      f->SetInputForegroundValue(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "BinaryImageToShapeLabelMapFilter",
    [&]() {
      auto f = itk::BinaryImageToShapeLabelMapFilter<FImg, SLM>::New();
      f->SetInput(binF);
      f->SetInputForegroundValue(1.0f);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryImageToShapeLabelMapFilter<DImg, SLM>::New();
      f->SetInput(binD);
      f->SetInputForegroundValue(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelImageToLabelMapFilter",
    [&]() {
      auto f = itk::LabelImageToLabelMapFilter<U16Img, LM>::New();
      f->SetInput(lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelImageToLabelMapFilter<U16Img, LM>::New();
      f->SetInput(lab16);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelMapToLabelImageFilter",
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<FImg, LM>::New();
      a->SetInput(binF);
      a->SetInputForegroundValue(1.0f);
      auto f = itk::LabelMapToLabelImageFilter<LM, U16Img>::New();
      f->SetInput(a->GetOutput());
      f->Update();
    },
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<DImg, LM>::New();
      a->SetInput(binD);
      a->SetInputForegroundValue(1.0);
      auto f = itk::LabelMapToLabelImageFilter<LM, U16Img>::New();
      f->SetInput(a->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelMapToBinaryImageFilter",
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<FImg, LM>::New();
      a->SetInput(binF);
      a->SetInputForegroundValue(1.0f);
      auto f = itk::LabelMapToBinaryImageFilter<LM, FImg>::New();
      f->SetInput(a->GetOutput());
      f->Update();
    },
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<DImg, LM>::New();
      a->SetInput(binD);
      a->SetInputForegroundValue(1.0);
      auto f = itk::LabelMapToBinaryImageFilter<LM, DImg>::New();
      f->SetInput(a->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "RelabelLabelMapFilter",
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<FImg, LM>::New();
      a->SetInput(binF);
      a->SetInputForegroundValue(1.0f);
      auto f = itk::RelabelLabelMapFilter<LM>::New();
      f->SetInput(a->GetOutput());
      f->Update();
    },
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<DImg, LM>::New();
      a->SetInput(binD);
      a->SetInputForegroundValue(1.0);
      auto f = itk::RelabelLabelMapFilter<LM>::New();
      f->SetInput(a->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ChangeLabelLabelMapFilter",
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<FImg, LM>::New();
      a->SetInput(binF);
      a->SetInputForegroundValue(1.0f);
      auto f = itk::ChangeLabelLabelMapFilter<LM>::New();
      f->SetInput(a->GetOutput());
      f->SetChange(1, 2);
      f->Update();
    },
    [&]() {
      auto a = itk::BinaryImageToLabelMapFilter<DImg, LM>::New();
      a->SetInput(binD);
      a->SetInputForegroundValue(1.0);
      auto f = itk::ChangeLabelLabelMapFilter<LM>::New();
      f->SetInput(a->GetOutput());
      f->SetChange(1, 2);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ShapeKeepNObjectsLabelMapFilter",
    [&]() {
      auto a = itk::BinaryImageToShapeLabelMapFilter<FImg, SLM>::New();
      a->SetInput(binF);
      a->SetInputForegroundValue(1.0f);
      auto f = itk::ShapeKeepNObjectsLabelMapFilter<SLM>::New();
      f->SetInput(a->GetOutput());
      f->SetNumberOfObjects(5);
      f->SetAttribute(SLO::NUMBER_OF_PIXELS);
      f->Update();
    },
    [&]() {
      auto a = itk::BinaryImageToShapeLabelMapFilter<DImg, SLM>::New();
      a->SetInput(binD);
      a->SetInputForegroundValue(1.0);
      auto f = itk::ShapeKeepNObjectsLabelMapFilter<SLM>::New();
      f->SetInput(a->GetOutput());
      f->SetNumberOfObjects(5);
      f->SetAttribute(SLO::NUMBER_OF_PIXELS);
      f->Update();
    },
    runs);

  BenchTime(
    "ImageGradient",
    "GradientImageFilter",
    [&]() {
      auto f = itk::GradientImageFilter<FImg>::New();
      f->SetInput(input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::GradientImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "VectorMagnitudeImageFilter",
    [&]() {
      auto g = itk::GradientImageFilter<FImg, float, float>::New();
      g->SetInput(input);
      using GM = itk::Image<itk::CovariantVector<float, 2>, 2>;
      auto f = itk::VectorMagnitudeImageFilter<GM, FImg>::New();
      f->SetInput(g->GetOutput());
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto g = itk::GradientImageFilter<DImg, double, double>::New();
      g->SetInput(c->GetOutput());
      using GM = itk::Image<itk::CovariantVector<double, 2>, 2>;
      auto f = itk::VectorMagnitudeImageFilter<GM, DImg>::New();
      f->SetInput(g->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageCompose",
    "ComposeImageFilter",
    [&]() {
      auto f = itk::ComposeImageFilter<FImg>::New();
      f->SetInput(0, input);
      f->SetInput(1, input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::ComposeImageFilter<DImg>::New();
      f->SetInput(0, c->GetOutput());
      f->SetInput(1, c->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageCompose",
    "JoinImageFilter",
    [&]() {
      auto f = itk::JoinImageFilter<FImg, FImg>::New();
      f->SetInput1(input);
      f->SetInput2(input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = itk::JoinImageFilter<DImg, DImg>::New();
      f->SetInput1(c->GetOutput());
      f->SetInput2(c->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "Colormap",
    "ScalarToRGBColormapImageFilter",
    [&]() {
      auto f = itk::ScalarToRGBColormapImageFilter<FImg, RGBImg>::New();
      f->SetInput(input);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::ScalarToRGBColormapImageFilter<DImg, RGBImg>::New();
      f->SetInput(c->GetOutput());
      f->Update();
    },
    runs);

  Bench(
    "DisplacementField",
    "DisplacementFieldJacobianDeterminantFilter",
    [&]() {
      auto f = itk::DisplacementFieldJacobianDeterminantFilter<VFImg, float, FImg>::New();
      f->SetInput(dispF);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      auto f = itk::DisplacementFieldJacobianDeterminantFilter<VDImg, double, DImg>::New();
      f->SetInput(dispD);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  BenchTime(
    "DisplacementField",
    "InvertDisplacementFieldImageFilter",
    [&]() {
      auto f = itk::InvertDisplacementFieldImageFilter<VFImg>::New();
      f->SetInput(dispF);
      f->SetMaximumNumberOfIterations(4);
      f->Update();
    },
    [&]() {
      auto f = itk::InvertDisplacementFieldImageFilter<VDImg>::New();
      f->SetInput(dispD);
      f->SetMaximumNumberOfIterations(4);
      f->Update();
    },
    runs);
  Bench(
    "ImageGrid",
    "WarpImageFilter",
    [&]() {
      auto f = itk::WarpImageFilter<FImg, FImg, VFImg>::New();
      f->SetInput(input);
      f->SetDisplacementField(dispF);
      f->SetOutputSpacing(input->GetSpacing());
      f->SetOutputOrigin(input->GetOrigin());
      f->SetOutputDirection(input->GetDirection());
      f->SetOutputSize(input->GetLargestPossibleRegion().GetSize());
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::WarpImageFilter<DImg, DImg, VDImg>::New();
      f->SetInput(c->GetOutput());
      f->SetDisplacementField(dispD);
      f->SetOutputSpacing(input->GetSpacing());
      f->SetOutputOrigin(input->GetOrigin());
      f->SetOutputDirection(input->GetDirection());
      f->SetOutputSize(input->GetLargestPossibleRegion().GetSize());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  BenchTime(
    "DistanceMap",
    "HausdorffDistanceImageFilter",
    [&]() {
      auto f = itk::HausdorffDistanceImageFilter<FImg, FImg>::New();
      f->SetInput1(binF);
      f->SetInput2(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::HausdorffDistanceImageFilter<DImg, DImg>::New();
      f->SetInput1(binD);
      f->SetInput2(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageCompare",
    "SimilarityIndexImageFilter",
    [&]() {
      auto f = itk::SimilarityIndexImageFilter<FImg, FImg>::New();
      f->SetInput1(binF);
      f->SetInput2(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::SimilarityIndexImageFilter<DImg, DImg>::New();
      f->SetInput1(binD);
      f->SetInput2(binD);
      f->Update();
    },
    runs);

  BenchTime(
    "PDEDeformable",
    "DemonsRegistrationFilter",
    [&]() {
      auto f = itk::DemonsRegistrationFilter<FImg, FImg, VFImg>::New();
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
      auto f = itk::DemonsRegistrationFilter<DImg, DImg, VDImg>::New();
      f->SetFixedImage(c->GetOutput());
      f->SetMovingImage(c->GetOutput());
      f->SetNumberOfIterations(4);
      f->Update();
    },
    runs);
}
