// 补测剩余能造数据的：形态学核、LabelMap、位移场、FFT/复数、向量/伪彩、N4/Hough 等。
// precision_remain_bench --list
// precision_remain_bench <image.png> [runs=3] [OperatorName]
#include "itkAggregateLabelMapFilter.h"
#include "itkAnchorCloseImageFilter.h"
#include "itkAnchorDilateImageFilter.h"
#include "itkAnchorErodeImageFilter.h"
#include "itkAnchorOpenImageFilter.h"
#include "itkAutoCropLabelMapFilter.h"
#include "itkBasicDilateImageFilter.h"
#include "itkBasicErodeImageFilter.h"
#include "itkBayesianClassifierImageFilter.h"
#include "itkBayesianClassifierInitializationImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkArray.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkBinaryImageToStatisticsLabelMapFilter.h"
#include "itkBinaryMagnitudeImageFilter.h"
#include "itkBinaryReconstructionByDilationImageFilter.h"
#include "itkBinaryReconstructionByErosionImageFilter.h"
#include "itkBinaryShapeOpeningImageFilter.h"
#include "itkBinaryStatisticsKeepNObjectsImageFilter.h"
#include "itkBinaryStatisticsOpeningImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBlockMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkChangeRegionLabelMapFilter.h"
#include "itkComplexToImaginaryImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include "itkComplexToPhaseImageFilter.h"
#include "itkComplexToRealImageFilter.h"
#include "itkComposeImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkCropLabelMapFilter.h"
#include "itkCurvatureRegistrationFilter.h"
#include "itkDiffeomorphicDemonsRegistrationFilter.h"
#include "itkDisplacementFieldToBSplineImageFilter.h"
#include "itkEdgePotentialImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkFrequencyBandImageFilter.h"
#include "itkFullToHalfHermitianImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkGrayscaleConnectedClosingImageFilter.h"
#include "itkGrayscaleConnectedOpeningImageFilter.h"
#include "itkGrayscaleFunctionDilateImageFilter.h"
#include "itkGrayscaleFunctionErodeImageFilter.h"
#include "itkGrayscaleGeodesicDilateImageFilter.h"
#include "itkGrayscaleGeodesicErodeImageFilter.h"
#include "itkGridImageSource.h"
#include "itkHalfToFullHermitianImageFilter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkHistogramThresholdImageFilter.h"
#include "itkHoughTransform2DLinesImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegistrationMethod.h"
#include "itkInterpolateImageFilter.h"
#include "itkIterativeInverseDisplacementFieldImageFilter.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkLabelImageToStatisticsLabelMapFilter.h"
#include "itkLabelMapMaskImageFilter.h"
#include "itkLabelMapOverlayImageFilter.h"
#include "itkLabelMapToAttributeImageFilter.h"
#include "itkLabelMapToRGBImageFilter.h"
#include "itkLabelOverlayImageFilter.h"
#include "itkLabelOverlapMeasuresImageFilter.h"
#include "itkLabelSelectionLabelMapFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkLabelShapeOpeningImageFilter.h"
#include "itkLabelStatisticsImageFilter.h"
#include "itkLabelStatisticsKeepNObjectsImageFilter.h"
#include "itkLabelStatisticsOpeningImageFilter.h"
#include "itkLabelToRGBImageFilter.h"
#include "itkLevelSetMotionRegistrationFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMagnitudeAndPhaseToComplexImageFilter.h"
#include "itkMaskedRankImageFilter.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMergeLabelMapFilter.h"
#include "itkMetaImageIOFactory.h"
#include "itkMovingHistogramDilateImageFilter.h"
#include "itkMovingHistogramErodeImageFilter.h"
#include "itkMovingHistogramMorphologicalGradientImageFilter.h"
#include "itkMultiLabelSTAPLEImageFilter.h"
#include "itkMultiThreaderBase.h"
#include "itkN4BiasFieldCorrectionImageFilter.h"
#include "itkOtsuThresholdCalculator.h"
#include "itkPNGImageIOFactory.h"
#include "itkPadImageFilter.h"
#include "itkPadLabelMapFilter.h"
#include "itkPoint.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkParametricBlindLeastSquaresDeconvolutionImageFilter.h"
#include "itkPatchBasedDenoisingImageFilter.h"
#include "itkPhysicalPointImageSource.h"
#include "itkPointSet.h"
#include "itkRGBToLuminanceImageFilter.h"
#include "itkReconstructionByDilationImageFilter.h"
#include "itkReconstructionByErosionImageFilter.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkRGBPixel.h"
#include "itkVector.h"
#include "itkShapeLabelObjectAccessors.h"
#include "itkShapeOpeningLabelMapFilter.h"
#include "itkShapeRelabelLabelMapFilter.h"
#include "itkShapeUniqueLabelMapFilter.h"
#include "itkShiftScaleLabelMapFilter.h"
#include "itkSliceImageFilter.h"
#include "itkStatisticsKeepNObjectsLabelMapFilter.h"
#include "itkStatisticsOpeningLabelMapFilter.h"
#include "itkStatisticsRelabelImageFilter.h"
#include "itkStatisticsRelabelLabelMapFilter.h"
#include "itkTernaryMagnitudeImageFilter.h"
#include "itkTernaryMagnitudeSquaredImageFilter.h"
#include "itkConvertLabelMapFilter.h"
#include "itkShapeLabelMapFilter.h"
#include "itkStatisticsLabelMapFilter.h"
#include "itkVectorGradientMagnitudeImageFilter.h"
#include "itkTransformToDisplacementFieldFilter.h"
#include "itkTranslationTransform.h"
#include "itkVanHerkGilWermanDilateImageFilter.h"
#include "itkVanHerkGilWermanErodeImageFilter.h"
#include "itkVectorCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkVectorGradientAnisotropicDiffusionImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkVectorRescaleIntensityImageFilter.h"
#include "itkVnlForward1DFFTImageFilter.h"
#include "itkVnlHalfHermitianToRealInverseFFTImageFilter.h"
#include "itkVnlInverse1DFFTImageFilter.h"
#include "itkVnlRealToHalfHermitianForwardFFTImageFilter.h"
#include "itkVoronoiPartitioningImageFilter.h"
#include "itkVoronoiSegmentationImageFilter.h"
#include "itkWarpVectorImageFilter.h"

#include <complex>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;
using U8Img = itk::Image<unsigned char, Dim>;
using U16Img = itk::Image<unsigned short, Dim>;
using Vec2F = itk::Vector<float, 2>;
using Vec2D = itk::Vector<double, 2>;
using VFImg = itk::Image<Vec2F, Dim>;
using VDImg = itk::Image<Vec2D, Dim>;
using CFImg = itk::Image<std::complex<float>, Dim>;
using CDImg = itk::Image<std::complex<double>, Dim>;
using RGBImg = itk::Image<itk::RGBPixel<unsigned char>, Dim>;
using Kernel = itk::FlatStructuringElement<Dim>;
using Trans = itk::TranslationTransform<double, Dim>;
using ShapeLO = itk::ShapeLabelObject<unsigned short, Dim>;
using ShapeLM = itk::LabelMap<ShapeLO>;
using StatLO = itk::StatisticsLabelObject<unsigned short, Dim>;
using StatLM = itk::LabelMap<StatLO>;

static std::string g_only;
static bool        g_list = false;

static bool
Want(const std::string & op)
{
  if (g_list)
  {
    std::cout << op << '\n';
    return false;
  }
  return g_only.empty() || g_only == op;
}

template <typename TImage>
static typename TImage::Pointer
Hold(TImage * p)
{
  typename TImage::Pointer out(p);
  out->DisconnectPipeline();
  return out;
}

template <typename Fn>
static double
TimeRuns(Fn fn, int runs)
{
  double t = 0.0;
  for (int i = 0; i < runs; ++i)
  {
    const auto t0 = Clock::now();
    fn();
    t += std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  }
  return t / static_cast<double>(runs);
}

static void
Emit(const std::string & module, const std::string & op, double msF, double msD)
{
  const double sp = msF > 0.0 ? (msD / msF) : 0.0;
  std::cout << std::fixed << std::setprecision(4) << module << ',' << op << ',' << msF << ',' << msD << ',' << sp
            << ",0,0" << std::endl;
}

template <typename FnF, typename FnD>
static void
BenchTime(const std::string & module, const std::string & op, FnF runF, FnD runD, int runs)
{
  if (!Want(op))
  {
    return;
  }
  std::cerr << ">> " << module << '/' << op << std::endl;
  try
  {
    runF();
    const double msF = TimeRuns(runF, runs);
    const double msD = TimeRuns(runD, runs);
    Emit(module, op, msF, msD);
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL," << e.GetDescription() << ",\n";
  }
  catch (const std::exception & e)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL," << e.what() << ",\n";
  }
  catch (...)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL,unknown,\n";
  }
}

template <typename TImage>
static typename TImage::Pointer
Bin(const TImage * in)
{
  using F = itk::BinaryThresholdImageFilter<TImage, TImage>;
  auto f = F::New();
  f->SetInput(in);
  f->SetLowerThreshold(40);
  f->SetUpperThreshold(255);
  f->SetInsideValue(1);
  f->SetOutsideValue(0);
  f->Update();
  return Hold(f->GetOutput());
}

template <typename TImage>
static typename TImage::Pointer
Shift(const TImage * in, double dx, double dy)
{
  using Res = itk::ResampleImageFilter<TImage, TImage>;
  auto t = Trans::New();
  typename Trans::OutputVectorType off;
  off[0] = dx;
  off[1] = dy;
  t->SetOffset(off);
  auto r = Res::New();
  r->SetInput(in);
  r->SetTransform(t);
  r->SetSize(in->GetLargestPossibleRegion().GetSize());
  r->SetOutputOrigin(in->GetOrigin());
  r->SetOutputSpacing(in->GetSpacing());
  r->SetOutputDirection(in->GetDirection());
  r->SetDefaultPixelValue(0);
  r->Update();
  return Hold(r->GetOutput());
}

template <typename TImage>
static typename TImage::Pointer
CropSq(const TImage * in, unsigned n)
{
  using F = itk::RegionOfInterestImageFilter<TImage, TImage>;
  auto                             f = F::New();
  typename TImage::RegionType      region;
  typename TImage::IndexType       idx;
  typename TImage::SizeType        sz;
  idx.Fill(0);
  sz.Fill(n);
  region.SetIndex(idx);
  region.SetSize(sz);
  f->SetRegionOfInterest(region);
  f->SetInput(in);
  f->Update();
  return Hold(f->GetOutput());
}

template <typename TImage>
static typename itk::Image<itk::Vector<typename TImage::PixelType, 2>, 2>::Pointer
MakeDisp(const TImage * ref)
{
  using Vec = itk::Vector<typename TImage::PixelType, 2>;
  using Out = itk::Image<Vec, 2>;
  auto o = Out::New();
  o->CopyInformation(ref);
  o->SetRegions(ref->GetLargestPossibleRegion());
  o->Allocate();
  Vec v;
  v[0] = static_cast<typename TImage::PixelType>(0.4);
  v[1] = static_cast<typename TImage::PixelType>(0.2);
  o->FillBuffer(v);
  return o;
}

template <typename TField>
static typename TField::Pointer
MakeVaryingDisp(const itk::ImageBase<2> * ref, unsigned n)
{
  auto                           o = TField::New();
  typename TField::RegionType    region;
  typename TField::IndexType     idx;
  typename TField::SizeType      sz;
  idx.Fill(0);
  sz.Fill(n);
  region.SetIndex(idx);
  region.SetSize(sz);
  o->SetRegions(region);
  o->SetSpacing(ref->GetSpacing());
  o->SetOrigin(ref->GetOrigin());
  o->SetDirection(ref->GetDirection());
  o->Allocate();
  itk::ImageRegionIterator<TField> it(o, region);
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto i = it.GetIndex();
    typename TField::PixelType v;
    v[0] = static_cast<typename TField::PixelType::ValueType>(0.02 * static_cast<double>(i[0]));
    v[1] = static_cast<typename TField::PixelType::ValueType>(0.01 * static_cast<double>(i[1]));
    it.Set(v);
  }
  return o;
}

int
main(int argc, char ** argv)
{
  if (argc >= 2 && std::string(argv[1]) == "--list")
  {
    g_list = true;
  }
  else if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " --list | <image.png> [runs=3] [OperatorName]\n";
    return 1;
  }
  itk::PNGImageIOFactory::RegisterOneFactory();
  itk::MetaImageIOFactory::RegisterOneFactory();
  const int runs = (!g_list && argc >= 3) ? std::stoi(argv[2]) : 3;
  if (!g_list && argc >= 4)
  {
    g_only = argv[3];
  }

  FImg::Pointer   input, movingF, binF, markerF;
  DImg::Pointer   inputD, movingD, binD, markerD;
  U16Img::Pointer lab16;
  VFImg::Pointer  dispF, dispVaryF;
  VDImg::Pointer  dispD, dispVaryD;
  ShapeLM::Pointer shapeMap;
  StatLM::Pointer  statMap;
  Kernel::RadiusType ballRad;
  ballRad.Fill(1);
  Kernel             ball = Kernel::Ball(ballRad);
  Kernel::RadiusType lineRad;
  lineRad.Fill(0);
  lineRad[0] = 2;
  Kernel           line = Kernel::Box(lineRad);
  FImg::Pointer    smallF, smallBinF, smallMoveF;
  DImg::Pointer    smallD, smallBinD, smallMoveD;
  CFImg::Pointer   specF, halfF;
  CDImg::Pointer   specD, halfD;

  if (!g_list)
  {
    using Reader = itk::ImageFileReader<FImg>;
    auto reader = Reader::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    input = Hold(reader->GetOutput());
    movingF = Shift(input.GetPointer(), 3.0, 2.0);
    using Cast = itk::CastImageFilter<FImg, DImg>;
    auto c = Cast::New();
    c->SetInput(input);
    c->Update();
    inputD = Hold(c->GetOutput());
    movingD = Shift(inputD.GetPointer(), 3.0, 2.0);
    binF = Bin(input.GetPointer());
    binD = Bin(inputD.GetPointer());
    using ErodeF = itk::BasicErodeImageFilter<FImg, FImg, Kernel>;
    auto er = ErodeF::New();
    er->SetInput(binF);
    er->SetKernel(ball);
    er->Update();
    markerF = Hold(er->GetOutput());
    using ErodeD = itk::BasicErodeImageFilter<DImg, DImg, Kernel>;
    auto erD = ErodeD::New();
    erD->SetInput(binD);
    erD->SetKernel(ball);
    erD->Update();
    markerD = Hold(erD->GetOutput());
    using CC = itk::ConnectedComponentImageFilter<FImg, U16Img>;
    auto cc = CC::New();
    cc->SetInput(binF);
    cc->Update();
    lab16 = Hold(cc->GetOutput());
    dispF = MakeDisp(input.GetPointer());
    dispD = MakeDisp(inputD.GetPointer());
    dispVaryF = MakeVaryingDisp<VFImg>(input.GetPointer(), 64);
    dispVaryD = MakeVaryingDisp<VDImg>(inputD.GetPointer(), 64);
    using ToShape = itk::LabelImageToShapeLabelMapFilter<U16Img, ShapeLM>;
    auto ts = ToShape::New();
    ts->SetInput(lab16);
    ts->Update();
    shapeMap = ts->GetOutput();
    shapeMap->DisconnectPipeline();
    using ToStat = itk::LabelImageToStatisticsLabelMapFilter<U16Img, FImg, StatLM>;
    auto st = ToStat::New();
    st->SetInput(lab16);
    st->SetFeatureImage(input);
    st->Update();
    statMap = st->GetOutput();
    statMap->DisconnectPipeline();
    smallF = CropSq(input.GetPointer(), 96);
    smallD = CropSq(inputD.GetPointer(), 96);
    smallBinF = Bin(smallF.GetPointer());
    smallBinD = Bin(smallD.GetPointer());
    smallMoveF = Shift(smallF.GetPointer(), 2.0, 1.0);
    smallMoveD = Shift(smallD.GetPointer(), 2.0, 1.0);
    {
      auto fwd = itk::VnlForward1DFFTImageFilter<FImg, CFImg>::New();
      fwd->SetInput(input);
      fwd->SetDirection(0);
      fwd->Update();
      specF = Hold(fwd->GetOutput());
      auto fwdD = itk::VnlForward1DFFTImageFilter<DImg, CDImg>::New();
      fwdD->SetInput(inputD);
      fwdD->SetDirection(0);
      fwdD->Update();
      specD = Hold(fwdD->GetOutput());
      auto hf = itk::VnlRealToHalfHermitianForwardFFTImageFilter<FImg, CFImg>::New();
      hf->SetInput(input);
      hf->Update();
      halfF = Hold(hf->GetOutput());
      auto hd = itk::VnlRealToHalfHermitianForwardFFTImageFilter<DImg, CDImg>::New();
      hd->SetInput(inputD);
      hd->Update();
      halfD = Hold(hd->GetOutput());
    }
    const auto sz = input->GetLargestPossibleRegion().GetSize();
    std::cerr << "remain " << sz[0] << 'x' << sz[1]
              << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << std::endl;
  }

#define MORPH3(name, Filt)                                                                                             \
  BenchTime(                                                                                                           \
    "MathematicalMorphology",                                                                                          \
    name,                                                                                                              \
    [&]() {                                                                                                            \
      auto f = itk::Filt<FImg, FImg, Kernel>::New();                                                                   \
      f->SetInput(input);                                                                                              \
      f->SetKernel(ball);                                                                                              \
      f->Update();                                                                                                     \
    },                                                                                                                 \
    [&]() {                                                                                                            \
      auto f = itk::Filt<DImg, DImg, Kernel>::New();                                                                   \
      f->SetInput(inputD);                                                                                             \
      f->SetKernel(ball);                                                                                              \
      f->Update();                                                                                                     \
    },                                                                                                                 \
    runs);
#define MORPH2(name, Filt, kern)                                                                                       \
  BenchTime(                                                                                                           \
    "MathematicalMorphology",                                                                                          \
    name,                                                                                                              \
    [&]() {                                                                                                            \
      auto f = itk::Filt<FImg, Kernel>::New();                                                                         \
      f->SetInput(input);                                                                                              \
      f->SetKernel(kern);                                                                                              \
      f->Update();                                                                                                     \
    },                                                                                                                 \
    [&]() {                                                                                                            \
      auto f = itk::Filt<DImg, Kernel>::New();                                                                         \
      f->SetInput(inputD);                                                                                             \
      f->SetKernel(kern);                                                                                              \
      f->Update();                                                                                                     \
    },                                                                                                                 \
    runs);

  MORPH2("AnchorDilateImageFilter", AnchorDilateImageFilter, ball);
  MORPH2("AnchorErodeImageFilter", AnchorErodeImageFilter, ball);
  MORPH2("AnchorOpenImageFilter", AnchorOpenImageFilter, ball);
  MORPH2("AnchorCloseImageFilter", AnchorCloseImageFilter, ball);
  MORPH3("BasicDilateImageFilter", BasicDilateImageFilter);
  MORPH3("BasicErodeImageFilter", BasicErodeImageFilter);
  MORPH3("GrayscaleFunctionDilateImageFilter", GrayscaleFunctionDilateImageFilter);
  MORPH3("GrayscaleFunctionErodeImageFilter", GrayscaleFunctionErodeImageFilter);
  MORPH3("MovingHistogramDilateImageFilter", MovingHistogramDilateImageFilter);
  MORPH3("MovingHistogramErodeImageFilter", MovingHistogramErodeImageFilter);
  MORPH3("MovingHistogramMorphologicalGradientImageFilter", MovingHistogramMorphologicalGradientImageFilter);
  MORPH2("VanHerkGilWermanDilateImageFilter", VanHerkGilWermanDilateImageFilter, line);
  MORPH2("VanHerkGilWermanErodeImageFilter", VanHerkGilWermanErodeImageFilter, line);
#undef MORPH3
#undef MORPH2

  BenchTime(
    "MathematicalMorphology",
    "GrayscaleConnectedOpeningImageFilter",
    [&]() {
      auto f = itk::GrayscaleConnectedOpeningImageFilter<FImg, FImg>::New();
      f->SetInput(input);
      FImg::IndexType s;
      s.Fill(512);
      f->SetSeed(s);
      f->Update();
    },
    [&]() {
      auto f = itk::GrayscaleConnectedOpeningImageFilter<DImg, DImg>::New();
      f->SetInput(inputD);
      DImg::IndexType s;
      s.Fill(512);
      f->SetSeed(s);
      f->Update();
    },
    runs);
  BenchTime(
    "MathematicalMorphology",
    "GrayscaleConnectedClosingImageFilter",
    [&]() {
      auto f = itk::GrayscaleConnectedClosingImageFilter<FImg, FImg>::New();
      f->SetInput(input);
      FImg::IndexType s;
      s.Fill(512);
      f->SetSeed(s);
      f->Update();
    },
    [&]() {
      auto f = itk::GrayscaleConnectedClosingImageFilter<DImg, DImg>::New();
      f->SetInput(inputD);
      DImg::IndexType s;
      s.Fill(512);
      f->SetSeed(s);
      f->Update();
    },
    runs);
  BenchTime(
    "MathematicalMorphology",
    "GrayscaleGeodesicDilateImageFilter",
    [&]() {
      auto f = itk::GrayscaleGeodesicDilateImageFilter<FImg, FImg>::New();
      f->SetInput(markerF);
      f->SetMaskImage(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::GrayscaleGeodesicDilateImageFilter<DImg, DImg>::New();
      f->SetInput(markerD);
      f->SetMaskImage(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "MathematicalMorphology",
    "GrayscaleGeodesicErodeImageFilter",
    [&]() {
      auto f = itk::GrayscaleGeodesicErodeImageFilter<FImg, FImg>::New();
      f->SetInput(binF);
      f->SetMaskImage(markerF);
      f->Update();
    },
    [&]() {
      auto f = itk::GrayscaleGeodesicErodeImageFilter<DImg, DImg>::New();
      f->SetInput(binD);
      f->SetMaskImage(markerD);
      f->Update();
    },
    runs);
  BenchTime(
    "MathematicalMorphology",
    "ReconstructionByDilationImageFilter",
    [&]() {
      auto f = itk::ReconstructionByDilationImageFilter<FImg, FImg>::New();
      f->SetMarkerImage(markerF);
      f->SetMaskImage(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::ReconstructionByDilationImageFilter<DImg, DImg>::New();
      f->SetMarkerImage(markerD);
      f->SetMaskImage(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "MathematicalMorphology",
    "ReconstructionByErosionImageFilter",
    [&]() {
      auto f = itk::ReconstructionByErosionImageFilter<FImg, FImg>::New();
      f->SetMarkerImage(binF);
      f->SetMaskImage(markerF);
      f->Update();
    },
    [&]() {
      auto f = itk::ReconstructionByErosionImageFilter<DImg, DImg>::New();
      f->SetMarkerImage(binD);
      f->SetMaskImage(markerD);
      f->Update();
    },
    runs);
  BenchTime(
    "MathematicalMorphology",
    "MaskedRankImageFilter",
    [&]() {
      auto f = itk::MaskedRankImageFilter<FImg, FImg, FImg>::New();
      f->SetInput(input);
      f->SetMaskImage(binF);
      f->SetRadius(1);
      f->Update();
    },
    [&]() {
      auto f = itk::MaskedRankImageFilter<DImg, DImg, DImg>::New();
      f->SetInput(inputD);
      f->SetMaskImage(binD);
      f->SetRadius(1);
      f->Update();
    },
    runs);

  BenchTime(
    "LabelMap",
    "BinaryShapeOpeningImageFilter",
    [&]() {
      auto f = itk::BinaryShapeOpeningImageFilter<FImg>::New();
      f->SetInput(binF);
      f->SetLambda(20);
      f->SetFullyConnected(true);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryShapeOpeningImageFilter<DImg>::New();
      f->SetInput(binD);
      f->SetLambda(20);
      f->SetFullyConnected(true);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "BinaryStatisticsOpeningImageFilter",
    [&]() {
      auto f = itk::BinaryStatisticsOpeningImageFilter<FImg, FImg>::New();
      f->SetInput(binF);
      f->SetFeatureImage(input);
      f->SetLambda(20);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryStatisticsOpeningImageFilter<DImg, DImg>::New();
      f->SetInput(binD);
      f->SetFeatureImage(inputD);
      f->SetLambda(20);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "BinaryStatisticsKeepNObjectsImageFilter",
    [&]() {
      auto f = itk::BinaryStatisticsKeepNObjectsImageFilter<FImg, FImg>::New();
      f->SetInput(binF);
      f->SetFeatureImage(input);
      f->SetNumberOfObjects(3);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryStatisticsKeepNObjectsImageFilter<DImg, DImg>::New();
      f->SetInput(binD);
      f->SetFeatureImage(inputD);
      f->SetNumberOfObjects(3);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelShapeOpeningImageFilter",
    [&]() {
      auto f = itk::LabelShapeOpeningImageFilter<U16Img>::New();
      f->SetInput(lab16);
      f->SetLambda(20);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelShapeOpeningImageFilter<U16Img>::New();
      f->SetInput(lab16);
      f->SetLambda(20);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelShapeKeepNObjectsImageFilter",
    [&]() {
      auto f = itk::LabelShapeKeepNObjectsImageFilter<U16Img>::New();
      f->SetInput(lab16);
      f->SetNumberOfObjects(3);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelShapeKeepNObjectsImageFilter<U16Img>::New();
      f->SetInput(lab16);
      f->SetNumberOfObjects(3);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelStatisticsOpeningImageFilter",
    [&]() {
      auto f = itk::LabelStatisticsOpeningImageFilter<U16Img, FImg>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(input);
      f->SetLambda(20);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelStatisticsOpeningImageFilter<U16Img, DImg>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(inputD);
      f->SetLambda(20);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelStatisticsKeepNObjectsImageFilter",
    [&]() {
      auto f = itk::LabelStatisticsKeepNObjectsImageFilter<U16Img, FImg>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(input);
      f->SetNumberOfObjects(3);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelStatisticsKeepNObjectsImageFilter<U16Img, DImg>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(inputD);
      f->SetNumberOfObjects(3);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "StatisticsRelabelImageFilter",
    [&]() {
      auto f = itk::StatisticsRelabelImageFilter<U16Img, FImg>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(input);
      f->Update();
    },
    [&]() {
      auto f = itk::StatisticsRelabelImageFilter<U16Img, DImg>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "BinaryReconstructionByDilationImageFilter",
    [&]() {
      auto f = itk::BinaryReconstructionByDilationImageFilter<FImg>::New();
      f->SetMarkerImage(markerF);
      f->SetMaskImage(binF);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryReconstructionByDilationImageFilter<DImg>::New();
      f->SetMarkerImage(markerD);
      f->SetMaskImage(binD);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "BinaryReconstructionByErosionImageFilter",
    [&]() {
      auto f = itk::BinaryReconstructionByErosionImageFilter<FImg>::New();
      f->SetMarkerImage(binF);
      f->SetMaskImage(markerF);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryReconstructionByErosionImageFilter<DImg>::New();
      f->SetMarkerImage(binD);
      f->SetMaskImage(markerD);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelImageToShapeLabelMapFilter",
    [&]() {
      auto f = itk::LabelImageToShapeLabelMapFilter<U16Img, ShapeLM>::New();
      f->SetInput(lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelImageToShapeLabelMapFilter<U16Img, ShapeLM>::New();
      f->SetInput(lab16);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelImageToStatisticsLabelMapFilter",
    [&]() {
      auto f = itk::LabelImageToStatisticsLabelMapFilter<U16Img, FImg, StatLM>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(input);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelImageToStatisticsLabelMapFilter<U16Img, DImg, StatLM>::New();
      f->SetInput(lab16);
      f->SetFeatureImage(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "BinaryImageToStatisticsLabelMapFilter",
    [&]() {
      auto f = itk::BinaryImageToStatisticsLabelMapFilter<FImg, FImg, StatLM>::New();
      f->SetInput(binF);
      f->SetFeatureImage(input);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryImageToStatisticsLabelMapFilter<DImg, DImg, StatLM>::New();
      f->SetInput(binD);
      f->SetFeatureImage(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ShapeOpeningLabelMapFilter",
    [&]() {
      auto f = itk::ShapeOpeningLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->SetLambda(20);
      f->Update();
    },
    [&]() {
      auto f = itk::ShapeOpeningLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->SetLambda(20);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ShapeRelabelLabelMapFilter",
    [&]() {
      auto f = itk::ShapeRelabelLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::ShapeRelabelLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ShapeUniqueLabelMapFilter",
    [&]() {
      auto f = itk::ShapeUniqueLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::ShapeUniqueLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "AutoCropLabelMapFilter",
    [&]() {
      auto f = itk::AutoCropLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::AutoCropLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ShiftScaleLabelMapFilter",
    [&]() {
      auto f = itk::ShiftScaleLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->SetShift(1);
      f->Update();
    },
    [&]() {
      auto f = itk::ShiftScaleLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->SetShift(1);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelSelectionLabelMapFilter",
    [&]() {
      auto f = itk::LabelSelectionLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->SetLabel(1);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelSelectionLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->SetLabel(1);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "MergeLabelMapFilter",
    [&]() {
      auto f = itk::MergeLabelMapFilter<ShapeLM>::New();
      f->SetInput(0, shapeMap->Clone());
      f->SetInput(1, shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::MergeLabelMapFilter<ShapeLM>::New();
      f->SetInput(0, shapeMap->Clone());
      f->SetInput(1, shapeMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "AggregateLabelMapFilter",
    [&]() {
      auto f = itk::AggregateLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::AggregateLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "PadLabelMapFilter",
    [&]() {
      auto f = itk::PadLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      ShapeLM::SizeType p;
      p.Fill(2);
      f->SetPadSize(p);
      f->Update();
    },
    [&]() {
      auto f = itk::PadLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      ShapeLM::SizeType p;
      p.Fill(2);
      f->SetPadSize(p);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "CropLabelMapFilter",
    [&]() {
      auto f = itk::CropLabelMapFilter<ShapeLM>::New();
      f->SetNumberOfWorkUnits(1);
      f->SetInput(shapeMap->Clone());
      ShapeLM::SizeType p;
      p.Fill(1);
      f->SetCropSize(p);
      f->Update();
    },
    [&]() {
      auto f = itk::CropLabelMapFilter<ShapeLM>::New();
      f->SetNumberOfWorkUnits(1);
      f->SetInput(shapeMap->Clone());
      ShapeLM::SizeType p;
      p.Fill(1);
      f->SetCropSize(p);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "StatisticsOpeningLabelMapFilter",
    [&]() {
      auto f = itk::StatisticsOpeningLabelMapFilter<StatLM>::New();
      f->SetInput(statMap->Clone());
      f->SetLambda(20);
      f->Update();
    },
    [&]() {
      auto f = itk::StatisticsOpeningLabelMapFilter<StatLM>::New();
      f->SetInput(statMap->Clone());
      f->SetLambda(20);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "StatisticsRelabelLabelMapFilter",
    [&]() {
      auto f = itk::StatisticsRelabelLabelMapFilter<StatLM>::New();
      f->SetInput(statMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::StatisticsRelabelLabelMapFilter<StatLM>::New();
      f->SetInput(statMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "StatisticsKeepNObjectsLabelMapFilter",
    [&]() {
      auto f = itk::StatisticsKeepNObjectsLabelMapFilter<StatLM>::New();
      f->SetInput(statMap->Clone());
      f->SetNumberOfObjects(3);
      f->Update();
    },
    [&]() {
      auto f = itk::StatisticsKeepNObjectsLabelMapFilter<StatLM>::New();
      f->SetInput(statMap->Clone());
      f->SetNumberOfObjects(3);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelMapMaskImageFilter",
    [&]() {
      auto f = itk::LabelMapMaskImageFilter<ShapeLM, FImg>::New();
      f->SetNumberOfWorkUnits(1);
      f->SetInput(shapeMap);
      f->SetFeatureImage(input);
      f->SetCrop(false);
      if (shapeMap->GetNumberOfLabelObjects() > 0)
      {
        f->SetLabel(shapeMap->GetNthLabelObject(0)->GetLabel());
      }
      f->Update();
    },
    [&]() {
      auto f = itk::LabelMapMaskImageFilter<ShapeLM, DImg>::New();
      f->SetNumberOfWorkUnits(1);
      f->SetInput(shapeMap);
      f->SetFeatureImage(inputD);
      f->SetCrop(false);
      if (shapeMap->GetNumberOfLabelObjects() > 0)
      {
        f->SetLabel(shapeMap->GetNthLabelObject(0)->GetLabel());
      }
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "LabelMapToAttributeImageFilter",
    [&]() {
      using Acc = itk::Functor::NumberOfPixelsLabelObjectAccessor<ShapeLO>;
      auto f = itk::LabelMapToAttributeImageFilter<ShapeLM, FImg, Acc>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      using Acc = itk::Functor::NumberOfPixelsLabelObjectAccessor<ShapeLO>;
      auto f = itk::LabelMapToAttributeImageFilter<ShapeLM, DImg, Acc>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);

  BenchTime(
    "DisplacementField",
    "IterativeInverseDisplacementFieldImageFilter",
    [&]() {
      auto f = itk::IterativeInverseDisplacementFieldImageFilter<VFImg, VFImg>::New();
      f->SetInput(dispF);
      f->SetNumberOfIterations(3);
      f->Update();
    },
    [&]() {
      auto f = itk::IterativeInverseDisplacementFieldImageFilter<VDImg, VDImg>::New();
      f->SetInput(dispD);
      f->SetNumberOfIterations(3);
      f->Update();
    },
    runs);
  BenchTime(
    "DisplacementField",
    "DisplacementFieldToBSplineImageFilter",
    [&]() {
      using F = itk::DisplacementFieldToBSplineImageFilter<VFImg>;
      auto f = F::New();
      f->SetDisplacementField(dispVaryF);
      f->UseInputFieldToDefineTheBSplineDomainOn();
      f->SetNumberOfFittingLevels(1);
      f->SetSplineOrder(3);
      typename F::ArrayType nc;
      nc.Fill(4);
      f->SetNumberOfControlPoints(nc);
      f->Update();
    },
    [&]() {
      using F = itk::DisplacementFieldToBSplineImageFilter<VDImg>;
      auto f = F::New();
      f->SetDisplacementField(dispVaryD);
      f->UseInputFieldToDefineTheBSplineDomainOn();
      f->SetNumberOfFittingLevels(1);
      f->SetSplineOrder(3);
      typename F::ArrayType nc;
      nc.Fill(4);
      f->SetNumberOfControlPoints(nc);
      f->Update();
    },
    runs);
  BenchTime(
    "DisplacementField",
    "TransformToDisplacementFieldFilter",
    [&]() {
      auto f = itk::TransformToDisplacementFieldFilter<VFImg, float>::New();
      auto t = itk::TranslationTransform<float, Dim>::New();
      t->SetIdentity();
      f->SetTransform(t.GetPointer());
      f->SetSize(input->GetLargestPossibleRegion().GetSize());
      f->SetOutputSpacing(input->GetSpacing());
      f->SetOutputOrigin(input->GetOrigin());
      f->Update();
    },
    [&]() {
      auto f = itk::TransformToDisplacementFieldFilter<VDImg, double>::New();
      auto t = Trans::New();
      t->SetIdentity();
      f->SetTransform(t);
      f->SetSize(inputD->GetLargestPossibleRegion().GetSize());
      f->SetOutputSpacing(inputD->GetSpacing());
      f->SetOutputOrigin(inputD->GetOrigin());
      f->Update();
    },
    runs);

  BenchTime(
    "FFT",
    "VnlForward1DFFTImageFilter",
    [&]() {
      auto f = itk::VnlForward1DFFTImageFilter<FImg, CFImg>::New();
      f->SetInput(input);
      f->SetDirection(0);
      f->Update();
    },
    [&]() {
      auto f = itk::VnlForward1DFFTImageFilter<DImg, CDImg>::New();
      f->SetInput(inputD);
      f->SetDirection(0);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "VnlInverse1DFFTImageFilter",
    [&]() {
      auto f = itk::VnlInverse1DFFTImageFilter<CFImg, FImg>::New();
      f->SetInput(specF);
      f->SetDirection(0);
      f->Update();
    },
    [&]() {
      auto f = itk::VnlInverse1DFFTImageFilter<CDImg, DImg>::New();
      f->SetInput(specD);
      f->SetDirection(0);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "VnlRealToHalfHermitianForwardFFTImageFilter",
    [&]() {
      auto f = itk::VnlRealToHalfHermitianForwardFFTImageFilter<FImg, CFImg>::New();
      f->SetInput(input);
      f->Update();
    },
    [&]() {
      auto f = itk::VnlRealToHalfHermitianForwardFFTImageFilter<DImg, CDImg>::New();
      f->SetInput(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "VnlHalfHermitianToRealInverseFFTImageFilter",
    [&]() {
      auto f = itk::VnlHalfHermitianToRealInverseFFTImageFilter<CFImg, FImg>::New();
      f->SetInput(halfF);
      f->SetActualXDimensionIsOdd(false);
      f->Update();
    },
    [&]() {
      auto f = itk::VnlHalfHermitianToRealInverseFFTImageFilter<CDImg, DImg>::New();
      f->SetInput(halfD);
      f->SetActualXDimensionIsOdd(false);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "FullToHalfHermitianImageFilter",
    [&]() {
      auto f = itk::FullToHalfHermitianImageFilter<CFImg>::New();
      f->SetInput(specF);
      f->Update();
    },
    [&]() {
      auto f = itk::FullToHalfHermitianImageFilter<CDImg>::New();
      f->SetInput(specD);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "HalfToFullHermitianImageFilter",
    [&]() {
      auto f = itk::HalfToFullHermitianImageFilter<CFImg>::New();
      f->SetInput(halfF);
      f->SetActualXDimensionIsOdd(false);
      f->Update();
    },
    [&]() {
      auto f = itk::HalfToFullHermitianImageFilter<CDImg>::New();
      f->SetInput(halfD);
      f->SetActualXDimensionIsOdd(false);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "ComplexToModulusImageFilter",
    [&]() {
      auto f = itk::ComplexToModulusImageFilter<CFImg, FImg>::New();
      f->SetInput(specF);
      f->Update();
    },
    [&]() {
      auto f = itk::ComplexToModulusImageFilter<CDImg, DImg>::New();
      f->SetInput(specD);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "ComplexToRealImageFilter",
    [&]() {
      auto f = itk::ComplexToRealImageFilter<CFImg, FImg>::New();
      f->SetInput(specF);
      f->Update();
    },
    [&]() {
      auto f = itk::ComplexToRealImageFilter<CDImg, DImg>::New();
      f->SetInput(specD);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "ComplexToImaginaryImageFilter",
    [&]() {
      auto f = itk::ComplexToImaginaryImageFilter<CFImg, FImg>::New();
      f->SetInput(specF);
      f->Update();
    },
    [&]() {
      auto f = itk::ComplexToImaginaryImageFilter<CDImg, DImg>::New();
      f->SetInput(specD);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "ComplexToPhaseImageFilter",
    [&]() {
      auto f = itk::ComplexToPhaseImageFilter<CFImg, FImg>::New();
      f->SetInput(specF);
      f->Update();
    },
    [&]() {
      auto f = itk::ComplexToPhaseImageFilter<CDImg, DImg>::New();
      f->SetInput(specD);
      f->Update();
    },
    runs);

  BenchTime(
    "AnisotropicSmoothing",
    "VectorGradientAnisotropicDiffusionImageFilter",
    [&]() {
      auto f = itk::VectorGradientAnisotropicDiffusionImageFilter<VFImg, VFImg>::New();
      f->SetInput(dispF);
      f->SetNumberOfIterations(4);
      f->SetTimeStep(0.0625);
      f->SetConductanceParameter(1.0);
      f->Update();
    },
    [&]() {
      auto f = itk::VectorGradientAnisotropicDiffusionImageFilter<VDImg, VDImg>::New();
      f->SetInput(dispD);
      f->SetNumberOfIterations(4);
      f->SetTimeStep(0.0625);
      f->SetConductanceParameter(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "AnisotropicSmoothing",
    "VectorCurvatureAnisotropicDiffusionImageFilter",
    [&]() {
      auto f = itk::VectorCurvatureAnisotropicDiffusionImageFilter<VFImg, VFImg>::New();
      f->SetInput(dispF);
      f->SetNumberOfIterations(4);
      f->SetTimeStep(0.0625);
      f->SetConductanceParameter(1.0);
      f->Update();
    },
    [&]() {
      auto f = itk::VectorCurvatureAnisotropicDiffusionImageFilter<VDImg, VDImg>::New();
      f->SetInput(dispD);
      f->SetNumberOfIterations(4);
      f->SetTimeStep(0.0625);
      f->SetConductanceParameter(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "VectorRescaleIntensityImageFilter",
    [&]() {
      auto f = itk::VectorRescaleIntensityImageFilter<VFImg, VFImg>::New();
      f->SetInput(dispF);
      f->SetOutputMaximumMagnitude(1.0);
      f->Update();
    },
    [&]() {
      auto f = itk::VectorRescaleIntensityImageFilter<VDImg, VDImg>::New();
      f->SetInput(dispD);
      f->SetOutputMaximumMagnitude(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "VectorIndexSelectionCastImageFilter",
    [&]() {
      auto f = itk::VectorIndexSelectionCastImageFilter<VFImg, FImg>::New();
      f->SetInput(dispF);
      f->SetIndex(0);
      f->Update();
    },
    [&]() {
      auto f = itk::VectorIndexSelectionCastImageFilter<VDImg, DImg>::New();
      f->SetInput(dispD);
      f->SetIndex(0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGrid",
    "WarpVectorImageFilter",
    [&]() {
      auto f = itk::WarpVectorImageFilter<VFImg, VFImg, VFImg>::New();
      f->SetInput(dispF);
      f->SetDisplacementField(dispF);
      f->SetOutputSpacing(input->GetSpacing());
      f->SetOutputOrigin(input->GetOrigin());
      f->SetOutputDirection(input->GetDirection());
      f->Update();
    },
    [&]() {
      auto f = itk::WarpVectorImageFilter<VDImg, VDImg, VDImg>::New();
      f->SetInput(dispD);
      f->SetDisplacementField(dispD);
      f->SetOutputSpacing(inputD->GetSpacing());
      f->SetOutputOrigin(inputD->GetOrigin());
      f->SetOutputDirection(inputD->GetDirection());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "TernaryMagnitudeImageFilter",
    [&]() {
      auto f = itk::TernaryMagnitudeImageFilter<FImg, FImg, FImg, FImg>::New();
      f->SetInput1(input);
      f->SetInput2(input);
      f->SetInput3(input);
      f->Update();
    },
    [&]() {
      auto f = itk::TernaryMagnitudeImageFilter<DImg, DImg, DImg, DImg>::New();
      f->SetInput1(inputD);
      f->SetInput2(inputD);
      f->SetInput3(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "BinaryMagnitudeImageFilter",
    [&]() {
      auto f = itk::BinaryMagnitudeImageFilter<FImg, FImg, FImg>::New();
      f->SetInput1(input);
      f->SetInput2(movingF);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryMagnitudeImageFilter<DImg, DImg, DImg>::New();
      f->SetInput1(inputD);
      f->SetInput2(movingD);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "EdgePotentialImageFilter",
    [&]() {
      using Grad = itk::GradientImageFilter<FImg>;
      auto g = Grad::New();
      g->SetInput(input);
      auto f = itk::EdgePotentialImageFilter<Grad::OutputImageType, FImg>::New();
      f->SetInput(g->GetOutput());
      f->Update();
    },
    [&]() {
      using Grad = itk::GradientImageFilter<DImg>;
      auto g = Grad::New();
      g->SetInput(inputD);
      auto f = itk::EdgePotentialImageFilter<Grad::OutputImageType, DImg>::New();
      f->SetInput(g->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFusion",
    "LabelToRGBImageFilter",
    [&]() {
      auto f = itk::LabelToRGBImageFilter<U16Img, RGBImg>::New();
      f->SetInput(lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelToRGBImageFilter<U16Img, RGBImg>::New();
      f->SetInput(lab16);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFusion",
    "LabelOverlayImageFilter",
    [&]() {
      auto f = itk::LabelOverlayImageFilter<FImg, U16Img, RGBImg>::New();
      f->SetInput(input);
      f->SetLabelImage(lab16);
      f->SetOpacity(0.4);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelOverlayImageFilter<DImg, U16Img, RGBImg>::New();
      f->SetInput(inputD);
      f->SetLabelImage(lab16);
      f->SetOpacity(0.4);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "RGBToLuminanceImageFilter",
    [&]() {
      auto rgb = itk::LabelToRGBImageFilter<U16Img, RGBImg>::New();
      rgb->SetInput(lab16);
      rgb->Update();
      auto img = Hold(rgb->GetOutput());
      auto f = itk::RGBToLuminanceImageFilter<RGBImg, FImg>::New();
      f->SetInput(img);
      f->Update();
    },
    [&]() {
      auto rgb = itk::LabelToRGBImageFilter<U16Img, RGBImg>::New();
      rgb->SetInput(lab16);
      rgb->Update();
      auto img = Hold(rgb->GetOutput());
      auto f = itk::RGBToLuminanceImageFilter<RGBImg, DImg>::New();
      f->SetInput(img);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGrid",
    "InterpolateImageFilter",
    [&]() {
      auto f = itk::InterpolateImageFilter<FImg, FImg>::New();
      f->SetInput1(input);
      f->SetInput2(movingF);
      f->SetDistance(0.5);
      f->Update();
    },
    [&]() {
      auto f = itk::InterpolateImageFilter<DImg, DImg>::New();
      f->SetInput1(inputD);
      f->SetInput2(movingD);
      f->SetDistance(0.5);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGrid",
    "SliceImageFilter",
    [&]() {
      auto f = itk::SliceImageFilter<FImg, FImg>::New();
      f->SetInput(input);
      f->SetStart(0);
      f->SetStop(static_cast<int>(input->GetLargestPossibleRegion().GetSize()[1]));
      f->SetStep(2);
      f->Update();
    },
    [&]() {
      auto f = itk::SliceImageFilter<DImg, DImg>::New();
      f->SetInput(inputD);
      f->SetStart(0);
      f->SetStop(static_cast<int>(inputD->GetLargestPossibleRegion().GetSize()[1]));
      f->SetStep(2);
      f->Update();
    },
    runs);

  BenchTime(
    "Thresholding",
    "HistogramThresholdImageFilter",
    [&]() {
      using F = itk::HistogramThresholdImageFilter<FImg, FImg>;
      using Calc = itk::OtsuThresholdCalculator<F::HistogramType, float>;
      auto f = F::New();
      f->SetInput(input);
      f->SetCalculator(Calc::New());
      f->SetInsideValue(1);
      f->SetOutsideValue(0);
      f->Update();
    },
    [&]() {
      using F = itk::HistogramThresholdImageFilter<DImg, DImg>;
      using Calc = itk::OtsuThresholdCalculator<F::HistogramType>;
      auto f = F::New();
      f->SetInput(inputD);
      f->SetCalculator(Calc::New());
      f->SetInsideValue(1);
      f->SetOutsideValue(0);
      f->Update();
    },
    runs);
  BenchTime(
    "BiasCorrection",
    "N4BiasFieldCorrectionImageFilter",
    [&]() {
      using F = itk::N4BiasFieldCorrectionImageFilter<FImg, U8Img, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetNumberOfFittingLevels(1);
      itk::Array<unsigned int> it(1);
      it[0] = 3;
      f->SetMaximumNumberOfIterations(it);
      f->Update();
    },
    [&]() {
      using F = itk::N4BiasFieldCorrectionImageFilter<DImg, U8Img, DImg>;
      auto f = F::New();
      f->SetInput(inputD);
      f->SetNumberOfFittingLevels(1);
      itk::Array<unsigned int> it(1);
      it[0] = 3;
      f->SetMaximumNumberOfIterations(it);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageSources",
    "GridImageSource",
    [&]() {
      auto f = itk::GridImageSource<FImg>::New();
      f->SetSize(input->GetLargestPossibleRegion().GetSize());
      f->SetSpacing(input->GetSpacing());
      f->SetOrigin(input->GetOrigin());
      f->Update();
    },
    [&]() {
      auto f = itk::GridImageSource<DImg>::New();
      f->SetSize(inputD->GetLargestPossibleRegion().GetSize());
      f->SetSpacing(inputD->GetSpacing());
      f->SetOrigin(inputD->GetOrigin());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageSources",
    "PhysicalPointImageSource",
    [&]() {
      using Out = itk::Image<itk::Point<float, 2>, 2>;
      auto f = itk::PhysicalPointImageSource<Out>::New();
      f->SetSize(input->GetLargestPossibleRegion().GetSize());
      f->SetSpacing(input->GetSpacing());
      f->SetOrigin(input->GetOrigin());
      f->Update();
    },
    [&]() {
      using Out = itk::Image<itk::Point<double, 2>, 2>;
      auto f = itk::PhysicalPointImageSource<Out>::New();
      f->SetSize(inputD->GetLargestPossibleRegion().GetSize());
      f->SetSpacing(inputD->GetSpacing());
      f->SetOrigin(inputD->GetOrigin());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFeature",
    "HoughTransform2DLinesImageFilter",
    [&]() {
      auto f = itk::HoughTransform2DLinesImageFilter<float, float>::New();
      f->SetInput(smallBinF);
      f->SetNumberOfLines(1);
      f->SetVariance(2.0);
      f->Update();
    },
    [&]() {
      auto f = itk::HoughTransform2DLinesImageFilter<double, double>::New();
      f->SetInput(smallBinD);
      f->SetNumberOfLines(1);
      f->SetVariance(2.0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFeature",
    "HessianToObjectnessMeasureImageFilter",
    [&]() {
      using HessF = itk::HessianRecursiveGaussianImageFilter<FImg>;
      auto h = HessF::New();
      h->SetInput(input);
      h->SetSigma(1.0);
      auto f = itk::HessianToObjectnessMeasureImageFilter<HessF::OutputImageType, FImg>::New();
      f->SetInput(h->GetOutput());
      f->SetObjectDimension(1);
      f->Update();
    },
    [&]() {
      using HessD = itk::HessianRecursiveGaussianImageFilter<DImg>;
      auto h = HessD::New();
      h->SetInput(inputD);
      h->SetSigma(1.0);
      auto f = itk::HessianToObjectnessMeasureImageFilter<HessD::OutputImageType, DImg>::New();
      f->SetInput(h->GetOutput());
      f->SetObjectDimension(1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "LabelOverlapMeasuresImageFilter",
    [&]() {
      auto f = itk::LabelOverlapMeasuresImageFilter<U16Img>::New();
      f->SetSourceImage(lab16);
      f->SetTargetImage(lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelOverlapMeasuresImageFilter<U16Img>::New();
      f->SetSourceImage(lab16);
      f->SetTargetImage(lab16);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageStatistics",
    "LabelStatisticsImageFilter",
    [&]() {
      auto f = itk::LabelStatisticsImageFilter<FImg, U16Img>::New();
      f->SetInput(input);
      f->SetLabelInput(lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelStatisticsImageFilter<DImg, U16Img>::New();
      f->SetInput(inputD);
      f->SetLabelInput(lab16);
      f->Update();
    },
    runs);

  BenchTime(
    "Common",
    "ImageRegistrationMethod",
    [&]() {
      using Metric = itk::MeanSquaresImageToImageMetric<FImg, FImg>;
      using Interp = itk::LinearInterpolateImageFunction<FImg, double>;
      using Method = itk::ImageRegistrationMethod<FImg, FImg>;
      auto method = Method::New();
      auto opt = itk::RegularStepGradientDescentOptimizer::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      method->SetMetric(Metric::New());
      method->SetOptimizer(opt);
      method->SetTransform(tr);
      method->SetInterpolator(Interp::New());
      method->SetFixedImage(input);
      method->SetMovingImage(movingF);
      method->SetFixedImageRegion(input->GetBufferedRegion());
      method->SetInitialTransformParameters(tr->GetParameters());
      opt->SetMaximumStepLength(2.0);
      opt->SetMinimumStepLength(0.1);
      opt->SetNumberOfIterations(8);
      method->Update();
    },
    [&]() {
      using Metric = itk::MeanSquaresImageToImageMetric<DImg, DImg>;
      using Interp = itk::LinearInterpolateImageFunction<DImg, double>;
      using Method = itk::ImageRegistrationMethod<DImg, DImg>;
      auto method = Method::New();
      auto opt = itk::RegularStepGradientDescentOptimizer::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      method->SetMetric(Metric::New());
      method->SetOptimizer(opt);
      method->SetTransform(tr);
      method->SetInterpolator(Interp::New());
      method->SetFixedImage(inputD);
      method->SetMovingImage(movingD);
      method->SetFixedImageRegion(inputD->GetBufferedRegion());
      method->SetInitialTransformParameters(tr->GetParameters());
      opt->SetMaximumStepLength(2.0);
      opt->SetMinimumStepLength(0.1);
      opt->SetNumberOfIterations(8);
      method->Update();
    },
    runs);
  BenchTime(
    "PDEDeformable",
    "DiffeomorphicDemonsRegistrationFilter",
    [&]() {
      auto f = itk::DiffeomorphicDemonsRegistrationFilter<FImg, FImg, VFImg>::New();
      f->SetFixedImage(input);
      f->SetMovingImage(movingF);
      f->SetNumberOfIterations(4);
      f->SetStandardDeviations(1.0);
      f->Update();
    },
    [&]() {
      auto f = itk::DiffeomorphicDemonsRegistrationFilter<DImg, DImg, VDImg>::New();
      f->SetFixedImage(inputD);
      f->SetMovingImage(movingD);
      f->SetNumberOfIterations(4);
      f->SetStandardDeviations(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "PDEDeformable",
    "LevelSetMotionRegistrationFilter",
    [&]() {
      auto f = itk::LevelSetMotionRegistrationFilter<FImg, FImg, VFImg>::New();
      f->SetFixedImage(input);
      f->SetMovingImage(movingF);
      f->SetNumberOfIterations(4);
      f->Update();
    },
    [&]() {
      auto f = itk::LevelSetMotionRegistrationFilter<DImg, DImg, VDImg>::New();
      f->SetFixedImage(inputD);
      f->SetMovingImage(movingD);
      f->SetNumberOfIterations(4);
      f->Update();
    },
    runs);
  BenchTime(
    "Common",
    "BlockMatchingImageFilter",
    [&]() {
      using BM = itk::BlockMatchingImageFilter<FImg>;
      auto f = BM::New();
      f->SetFixedImage(smallF);
      f->SetMovingImage(smallMoveF);
      FImg::SizeType br;
      br.Fill(2);
      f->SetBlockRadius(br);
      FImg::SizeType sr;
      sr.Fill(3);
      f->SetSearchRadius(sr);
      auto pts = BM::FeaturePointsType::New();
      const auto origin = smallF->GetOrigin();
      const auto spacing = smallF->GetSpacing();
      typename BM::FeaturePointsType::PointIdentifier id = 0;
      for (int y = 16; y <= 80; y += 16)
      {
        for (int x = 16; x <= 80; x += 16)
        {
          typename BM::FeaturePointsPhysicalCoordinates p;
          p[0] = origin[0] + static_cast<double>(x) * spacing[0];
          p[1] = origin[1] + static_cast<double>(y) * spacing[1];
          pts->SetPoint(id++, p);
        }
      }
      f->SetFeaturePoints(pts);
      f->Update();
    },
    [&]() {
      using BM = itk::BlockMatchingImageFilter<DImg>;
      auto f = BM::New();
      f->SetFixedImage(smallD);
      f->SetMovingImage(smallMoveD);
      DImg::SizeType br;
      br.Fill(2);
      f->SetBlockRadius(br);
      DImg::SizeType sr;
      sr.Fill(3);
      f->SetSearchRadius(sr);
      auto pts = BM::FeaturePointsType::New();
      const auto origin = smallD->GetOrigin();
      const auto spacing = smallD->GetSpacing();
      typename BM::FeaturePointsType::PointIdentifier id = 0;
      for (int y = 16; y <= 80; y += 16)
      {
        for (int x = 16; x <= 80; x += 16)
        {
          typename BM::FeaturePointsPhysicalCoordinates p;
          p[0] = origin[0] + static_cast<double>(x) * spacing[0];
          p[1] = origin[1] + static_cast<double>(y) * spacing[1];
          pts->SetPoint(id++, p);
        }
      }
      f->SetFeaturePoints(pts);
      f->Update();
    },
    runs);

  BenchTime(
    "Denoising",
    "PatchBasedDenoisingImageFilter",
    [&]() {
      auto f = itk::PatchBasedDenoisingImageFilter<FImg, FImg>::New();
      f->SetInput(smallF);
      f->SetNumberOfIterations(1);
      f->SetPatchRadius(1);
      f->SetKernelBandwidthEstimation(false);
      itk::Array<double> sig(1);
      sig[0] = 400.0;
      f->SetKernelBandwidthSigma(sig);
      f->Update();
    },
    [&]() {
      auto f = itk::PatchBasedDenoisingImageFilter<DImg, DImg>::New();
      f->SetInput(smallD);
      f->SetNumberOfIterations(1);
      f->SetPatchRadius(1);
      f->SetKernelBandwidthEstimation(false);
      itk::Array<double> sig(1);
      sig[0] = 400.0;
      f->SetKernelBandwidthSigma(sig);
      f->Update();
    },
    runs);
  BenchTime(
    "Voronoi",
    "VoronoiSegmentationImageFilter",
    [&]() {
      auto f = itk::VoronoiSegmentationImageFilter<FImg, FImg>::New();
      f->SetInput(smallF);
      f->SetMeanPercentError(0.4);
      f->SetSTDPercentError(0.8);
      f->Update();
    },
    [&]() {
      auto f = itk::VoronoiSegmentationImageFilter<DImg, DImg>::New();
      f->SetInput(smallD);
      f->SetMeanPercentError(0.4);
      f->SetSTDPercentError(0.8);
      f->Update();
    },
    runs);
  BenchTime(
    "Voronoi",
    "VoronoiPartitioningImageFilter",
    [&]() {
      auto f = itk::VoronoiPartitioningImageFilter<FImg, FImg>::New();
      f->SetInput(smallF);
      f->Update();
    },
    [&]() {
      auto f = itk::VoronoiPartitioningImageFilter<DImg, DImg>::New();
      f->SetInput(smallD);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelVoting",
    "MultiLabelSTAPLEImageFilter",
    [&]() {
      auto f = itk::MultiLabelSTAPLEImageFilter<U16Img, U16Img>::New();
      f->SetInput(0, lab16);
      f->SetInput(1, lab16);
      f->Update();
    },
    [&]() {
      auto f = itk::MultiLabelSTAPLEImageFilter<U16Img, U16Img>::New();
      f->SetInput(0, lab16);
      f->SetInput(1, lab16);
      f->Update();
    },
    runs);
  BenchTime(
    "Classifiers",
    "BayesianClassifierInitializationImageFilter",
    [&]() {
      auto f = itk::BayesianClassifierInitializationImageFilter<FImg>::New();
      f->SetInput(input);
      f->SetNumberOfClasses(3);
      f->Update();
    },
    [&]() {
      auto f = itk::BayesianClassifierInitializationImageFilter<DImg>::New();
      f->SetInput(inputD);
      f->SetNumberOfClasses(3);
      f->Update();
    },
    runs);

  BenchTime(
    "ImageIntensity",
    "TernaryMagnitudeSquaredImageFilter",
    [&]() {
      auto f = itk::TernaryMagnitudeSquaredImageFilter<FImg, FImg, FImg, FImg>::New();
      f->SetInput1(input);
      f->SetInput2(input);
      f->SetInput3(input);
      f->Update();
    },
    [&]() {
      auto f = itk::TernaryMagnitudeSquaredImageFilter<DImg, DImg, DImg, DImg>::New();
      f->SetInput1(inputD);
      f->SetInput2(inputD);
      f->SetInput3(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "MagnitudeAndPhaseToComplexImageFilter",
    [&]() {
      auto f = itk::MagnitudeAndPhaseToComplexImageFilter<FImg, FImg, CFImg>::New();
      f->SetInput1(input);
      f->SetInput2(input);
      f->Update();
    },
    [&]() {
      auto f = itk::MagnitudeAndPhaseToComplexImageFilter<DImg, DImg, CDImg>::New();
      f->SetInput1(inputD);
      f->SetInput2(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGrid",
    "PadImageFilter",
    [&]() {
      auto f = itk::ConstantPadImageFilter<FImg, FImg>::New();
      f->SetInput(input);
      FImg::SizeType p;
      p.Fill(4);
      f->SetPadLowerBound(p);
      f->SetPadUpperBound(p);
      f->SetConstant(0);
      f->Update();
    },
    [&]() {
      auto f = itk::ConstantPadImageFilter<DImg, DImg>::New();
      f->SetInput(inputD);
      DImg::SizeType p;
      p.Fill(4);
      f->SetPadLowerBound(p);
      f->SetPadUpperBound(p);
      f->SetConstant(0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGradient",
    "VectorGradientMagnitudeImageFilter",
    [&]() {
      auto f = itk::VectorGradientMagnitudeImageFilter<VFImg>::New();
      f->SetInput(dispF);
      f->Update();
    },
    [&]() {
      auto f = itk::VectorGradientMagnitudeImageFilter<VDImg>::New();
      f->SetInput(dispD);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFrequency",
    "FrequencyBandImageFilter",
    [&]() {
      auto f = itk::FrequencyBandImageFilter<CFImg>::New();
      f->SetInput(specF);
      f->SetPassBand(0.0, 0.4);
      f->Update();
    },
    [&]() {
      auto f = itk::FrequencyBandImageFilter<CDImg>::New();
      f->SetInput(specD);
      f->SetPassBand(0.0, 0.4);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFusion",
    "LabelMapOverlayImageFilter",
    [&]() {
      auto f = itk::LabelMapOverlayImageFilter<ShapeLM, FImg, RGBImg>::New();
      f->SetInput(shapeMap->Clone());
      f->SetFeatureImage(input);
      f->SetOpacity(0.4);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelMapOverlayImageFilter<ShapeLM, DImg, RGBImg>::New();
      f->SetInput(shapeMap->Clone());
      f->SetFeatureImage(inputD);
      f->SetOpacity(0.4);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFusion",
    "LabelMapToRGBImageFilter",
    [&]() {
      auto f = itk::LabelMapToRGBImageFilter<ShapeLM, RGBImg>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::LabelMapToRGBImageFilter<ShapeLM, RGBImg>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ChangeRegionLabelMapFilter",
    [&]() {
      auto f = itk::ChangeRegionLabelMapFilter<ShapeLM>::New();
      auto in = shapeMap->Clone();
      f->SetInput(in);
      f->SetRegion(in->GetLargestPossibleRegion());
      f->Update();
    },
    [&]() {
      auto f = itk::ChangeRegionLabelMapFilter<ShapeLM>::New();
      auto in = shapeMap->Clone();
      f->SetInput(in);
      f->SetRegion(in->GetLargestPossibleRegion());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ShapeLabelMapFilter",
    [&]() {
      auto f = itk::ShapeLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::ShapeLabelMapFilter<ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "StatisticsLabelMapFilter",
    [&]() {
      auto f = itk::StatisticsLabelMapFilter<StatLM, FImg>::New();
      f->SetInput(statMap->Clone());
      f->SetFeatureImage(input);
      f->Update();
    },
    [&]() {
      auto f = itk::StatisticsLabelMapFilter<StatLM, DImg>::New();
      f->SetInput(statMap->Clone());
      f->SetFeatureImage(inputD);
      f->Update();
    },
    runs);
  BenchTime(
    "LabelMap",
    "ConvertLabelMapFilter",
    [&]() {
      auto f = itk::ConvertLabelMapFilter<ShapeLM, ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    [&]() {
      auto f = itk::ConvertLabelMapFilter<ShapeLM, ShapeLM>::New();
      f->SetInput(shapeMap->Clone());
      f->Update();
    },
    runs);

  return 0;
}
