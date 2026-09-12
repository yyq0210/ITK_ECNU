// 补测 2D Filter：float vs full_double。每算子可单独跑。
// precision_fill_missing_bench --list
// precision_fill_missing_bench <image.png> [runs=3] [OperatorName]
#include "itkAbsImageFilter.h"
#include "itkAbsoluteValueDifferenceImageFilter.h"
#include "itkAcosImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkAndImageFilter.h"
#include "itkAsinImageFilter.h"
#include "itkAtan2ImageFilter.h"
#include "itkAtanImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBlackTopHatImageFilter.h"
#include "itkBoundedReciprocalImageFilter.h"
#include "itkBoxMeanImageFilter.h"
#include "itkBoxSigmaImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCheckerBoardImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkConvolutionImageFilter.h"
#include "itkCosImageFilter.h"
#include "itkDanielssonDistanceMapImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkDivideImageFilter.h"
#include "itkDivideOrZeroOutImageFilter.h"
#include "itkEdgePotentialImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkExpNegativeImageFilter.h"
#include "itkFFTConvolutionImageFilter.h"
#include "itkFFTPadImageFilter.h"
#include "itkFFTShiftImageFilter.h"
#include "itkGaussianImageSource.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkGrayscaleDilateImageFilter.h"
#include "itkGrayscaleErodeImageFilter.h"
#include "itkGrayscaleMorphologicalClosingImageFilter.h"
#include "itkGrayscaleMorphologicalOpeningImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkLaplacianImageFilter.h"
#include "itkLaplacianRecursiveGaussianImageFilter.h"
#include "itkLaplacianSharpeningImageFilter.h"
#include "itkMaximumImageFilter.h"
#include "itkMeanImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkMetaImageIOFactory.h"
#include "itkMinimumImageFilter.h"
#include "itkMorphologicalGradientImageFilter.h"
#include "itkMultiThreaderBase.h"
#include "itkMultiplyImageFilter.h"
#include "itkNormalizeImageFilter.h"
#include "itkOrImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkPNGImageIOFactory.h"
#include "itkPowImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRoundImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkSinImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkSobelEdgeDetectionImageFilter.h"
#include "itkSqrtImageFilter.h"
#include "itkSquareImageFilter.h"
#include "itkSquaredDifferenceImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkTanImageFilter.h"
#include "itkWhiteTopHatImageFilter.h"
#include "itkXorImageFilter.h"
#include "itkZeroCrossingImageFilter.h"
#include "itkOffset.h"
#include "precision_fill_extra_includes.inc"
#include "precision_fill_construct2_includes.inc"
#include "precision_fill_construct3_includes.inc"

#include <chrono>
#include <cmath>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;
using BallF = itk::BinaryBallStructuringElement<float, Dim>;
using BallD = itk::BinaryBallStructuringElement<double, Dim>;

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

static void
RegisterIO()
{
  itk::PNGImageIOFactory::RegisterOneFactory();
  itk::MetaImageIOFactory::RegisterOneFactory();
}

struct DiffResult
{
  double maxAbs{ 0.0 };
  double rmse{ 0.0 };
};

static DiffResult
DiffFD(const DImg * ref, const FImg * test)
{
  DiffResult r;
  if (!ref || !test)
  {
    return r;
  }
  if (ref->GetLargestPossibleRegion().GetSize() != test->GetLargestPossibleRegion().GetSize())
  {
    return r;
  }
  double        sumSq = 0.0;
  unsigned long n = 0;
  itk::ImageRegionConstIterator<DImg> itR(ref, ref->GetBufferedRegion());
  itk::ImageRegionConstIterator<FImg> itT(test, test->GetBufferedRegion());
  for (itR.GoToBegin(), itT.GoToBegin(); !itR.IsAtEnd() && !itT.IsAtEnd(); ++itR, ++itT)
  {
    const double d = std::fabs(itR.Get() - static_cast<double>(itT.Get()));
    sumSq += d * d;
    r.maxAbs = std::max(r.maxAbs, d);
    ++n;
  }
  r.rmse = n > 0 ? std::sqrt(sumSq / static_cast<double>(n)) : 0.0;
  return r;
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
Emit(const std::string & module, const std::string & op, double msF, double msD, const DiffResult & d)
{
  const double sp = msF > 0.0 ? (msD / msF) : 0.0;
  std::cout << std::fixed << std::setprecision(4) << module << ',' << op << ',' << msF << ',' << msD << ',' << sp << ','
            << std::setprecision(8) << d.maxAbs << ',' << d.rmse << std::endl;
}

template <typename FnF, typename FnD>
static void
Bench(const std::string & module, const std::string & op, FnF runF, FnD runD, int runs)
{
  if (!Want(op))
  {
    return;
  }
  std::cerr << ">> " << module << '/' << op << std::endl;
  try
  {
    runF();
    const double     msF = TimeRuns(runF, runs);
    const double     msD = TimeRuns(runD, runs);
    const DiffResult d = DiffFD(runD().GetPointer(), runF().GetPointer());
    Emit(module, op, msF, msD, d);
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
    Emit(module, op, msF, msD, DiffResult{});
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

#define UNARY(mod, cls)                                                                                     \
  Bench(                                                                                                    \
    mod,                                                                                                    \
    #cls,                                                                                                   \
    [&]() {                                                                                                 \
      using F = itk::cls<FImg, FImg>;                                                                       \
      auto f = F::New();                                                                                    \
      f->SetInput(input);                                                                                   \
      f->Update();                                                                                          \
      return Hold(f->GetOutput());                                                                          \
    },                                                                                                      \
    [&]() {                                                                                                 \
      using Cast = itk::CastImageFilter<FImg, DImg>;                                                        \
      using F = itk::cls<DImg, DImg>;                                                                       \
      auto c = Cast::New();                                                                                 \
      c->SetInput(input);                                                                                   \
      auto f = F::New();                                                                                    \
      f->SetInput(c->GetOutput());                                                                          \
      f->Update();                                                                                          \
      return Hold(f->GetOutput());                                                                          \
    },                                                                                                      \
    runs)

#define BINARY_SELF(mod, cls)                                                                               \
  Bench(                                                                                                    \
    mod,                                                                                                    \
    #cls,                                                                                                   \
    [&]() {                                                                                                 \
      using F = itk::cls<FImg, FImg, FImg>;                                                                 \
      auto f = F::New();                                                                                    \
      f->SetInput1(input);                                                                                  \
      f->SetInput2(input);                                                                                  \
      f->Update();                                                                                          \
      return Hold(f->GetOutput());                                                                          \
    },                                                                                                      \
    [&]() {                                                                                                 \
      using Cast = itk::CastImageFilter<FImg, DImg>;                                                        \
      using F = itk::cls<DImg, DImg, DImg>;                                                                 \
      auto c = Cast::New();                                                                                 \
      c->SetInput(input);                                                                                   \
      c->Update();                                                                                          \
      auto f = F::New();                                                                                    \
      f->SetInput1(c->GetOutput());                                                                         \
      f->SetInput2(c->GetOutput());                                                                         \
      f->Update();                                                                                          \
      return Hold(f->GetOutput());                                                                          \
    },                                                                                                      \
    runs)

#define MORPH(mod, cls)                                                                                     \
  Bench(                                                                                                    \
    mod,                                                                                                    \
    #cls,                                                                                                   \
    [&]() {                                                                                                 \
      using F = itk::cls<FImg, FImg, BallF>;                                                                \
      auto f = F::New();                                                                                    \
      f->SetInput(input);                                                                                   \
      f->SetKernel(kf);                                                                                     \
      f->Update();                                                                                          \
      return Hold(f->GetOutput());                                                                          \
    },                                                                                                      \
    [&]() {                                                                                                 \
      using Cast = itk::CastImageFilter<FImg, DImg>;                                                        \
      using F = itk::cls<DImg, DImg, BallD>;                                                                \
      auto c = Cast::New();                                                                                 \
      c->SetInput(input);                                                                                   \
      auto f = F::New();                                                                                    \
      f->SetInput(c->GetOutput());                                                                          \
      f->SetKernel(kd);                                                                                     \
      f->Update();                                                                                          \
      return Hold(f->GetOutput());                                                                          \
    },                                                                                                      \
    runs)

template <typename FF, typename FD, typename SF, typename SD>
static void
Run2(const char * mod, const char * name, const typename FImg::Pointer & in, int runs, SF sf, SD sd)
{
  Bench(
    mod,
    name,
    [&]() {
      auto f = FF::New();
      f->SetInput(in);
      sf(f.GetPointer());
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(in);
      auto f = FD::New();
      f->SetInput(c->GetOutput());
      sd(f.GetPointer());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
}

template <typename FF, typename FD, typename SF, typename SD>
static void
Run1(const char * mod, const char * name, const typename FImg::Pointer & in, int runs, SF sf, SD sd)
{
  Run2<FF, FD>(mod, name, in, runs, sf, sd);
}

template <typename FF, typename FD, typename SF, typename SD>
static void
RunBin(const char * mod, const char * name, const typename FImg::Pointer & bf, const typename DImg::Pointer & bd, int runs, SF sf, SD sd)
{
  Bench(
    mod,
    name,
    [&]() {
      auto f = FF::New();
      f->SetInput(bf);
      sf(f.GetPointer());
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      auto f = FD::New();
      f->SetInput(bd);
      sd(f.GetPointer());
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
}

template <typename FF, typename FD, typename SF, typename SD>
static void
RunBin1(const char * mod, const char * name, const typename FImg::Pointer & bf, const typename DImg::Pointer & bd, int runs, SF sf, SD sd)
{
  RunBin<FF, FD>(mod, name, bf, bd, runs, sf, sd);
}

template <typename FF, typename FD>
static void
RunMorph(const char * mod, const char * name, const typename FImg::Pointer & in, const BallF & kf, const BallD & kd, int runs)
{
  Bench(
    mod,
    name,
    [&]() {
      auto f = FF::New();
      f->SetInput(in);
      f->SetKernel(kf);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(in);
      auto f = FD::New();
      f->SetInput(c->GetOutput());
      f->SetKernel(kd);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
}

template <typename FF, typename FD>
static void
RunBinMorph(const char * mod, const char * name, const typename FImg::Pointer & bf, const typename DImg::Pointer & bd, const BallF & kf, const BallD & kd, int runs)
{
  Bench(
    mod,
    name,
    [&]() {
      auto f = FF::New();
      f->SetInput(bf);
      f->SetKernel(kf);
      f->SetForegroundValue(1.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      auto f = FD::New();
      f->SetInput(bd);
      f->SetKernel(kd);
      f->SetForegroundValue(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
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

  RegisterIO();
  const int runs = (!g_list && argc >= 3) ? std::stoi(argv[2]) : 3;
  if (!g_list && argc >= 4)
  {
    g_only = argv[3];
  }

  FImg::Pointer  input;
  FImg::Pointer  binF;
  DImg::Pointer  binD;
  BallF          kf;
  BallD          kd;
  FImg::SizeType radius;
  radius.Fill(1);
  FImg::SizeType pad;
  pad.Fill(4);
  FImg::IndexType seed;
  seed.Fill(0);
  FImg::RegionType roi;

  if (!g_list)
  {
    using Reader = itk::ImageFileReader<FImg>;
    auto reader = Reader::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    input = reader->GetOutput();
    using Otsu = itk::OtsuThresholdImageFilter<FImg, FImg>;
    auto o = Otsu::New();
    o->SetInput(input);
    o->SetInsideValue(1.0f);
    o->SetOutsideValue(0.0f);
    o->Update();
    binF = o->GetOutput();
    using CastB = itk::CastImageFilter<FImg, DImg>;
    auto cb = CastB::New();
    cb->SetInput(binF);
    cb->Update();
    binD = cb->GetOutput();
    binD->DisconnectPipeline();
    kf.SetRadius(1);
    kf.CreateStructuringElement();
    kd.SetRadius(1);
    kd.CreateStructuringElement();
    const auto sz = input->GetLargestPossibleRegion().GetSize();
    seed[0] = static_cast<long>(sz[0] / 2);
    seed[1] = static_cast<long>(sz[1] / 2);
    {
      FImg::IndexType idx = input->GetLargestPossibleRegion().GetIndex();
      FImg::SizeType  rsz = sz;
      const long crop = 16;
      idx[0] += crop;
      idx[1] += crop;
      rsz[0] -= static_cast<unsigned long>(2 * crop);
      rsz[1] -= static_cast<unsigned long>(2 * crop);
      roi.SetIndex(idx);
      roi.SetSize(rsz);
    }
    std::cerr << "loaded " << sz[0] << 'x' << sz[1] << " runs=" << runs
              << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << std::endl;
  }

  UNARY("ImageIntensity", AbsImageFilter);
  UNARY("ImageIntensity", AcosImageFilter);
  UNARY("ImageIntensity", AsinImageFilter);
  UNARY("ImageIntensity", AtanImageFilter);
  UNARY("ImageIntensity", SinImageFilter);
  UNARY("ImageIntensity", CosImageFilter);
  UNARY("ImageIntensity", TanImageFilter);
  UNARY("ImageIntensity", ExpImageFilter);
  UNARY("ImageIntensity", ExpNegativeImageFilter);
  UNARY("ImageIntensity", SqrtImageFilter);
  UNARY("ImageIntensity", SquareImageFilter);
  UNARY("ImageIntensity", InvertIntensityImageFilter);
  UNARY("ImageIntensity", BoundedReciprocalImageFilter);
  UNARY("ImageIntensity", RoundImageFilter);
  UNARY("ImageIntensity", NormalizeImageFilter);
  BINARY_SELF("ImageIntensity", AddImageFilter);
  BINARY_SELF("ImageIntensity", SubtractImageFilter);
  BINARY_SELF("ImageIntensity", MultiplyImageFilter);
  BINARY_SELF("ImageIntensity", DivideImageFilter);
  BINARY_SELF("ImageIntensity", DivideOrZeroOutImageFilter);
  BINARY_SELF("ImageIntensity", MaximumImageFilter);
  BINARY_SELF("ImageIntensity", MinimumImageFilter);
  BINARY_SELF("ImageIntensity", Atan2ImageFilter);
  BINARY_SELF("ImageIntensity", PowImageFilter);

  UNARY("ImageFeature", LaplacianImageFilter);
  UNARY("ImageFeature", LaplacianSharpeningImageFilter);
  UNARY("ImageFeature", ZeroCrossingImageFilter);
  UNARY("ImageFeature", SobelEdgeDetectionImageFilter);
  UNARY("ImageGradient", GradientMagnitudeImageFilter);
  UNARY("FFT", FFTShiftImageFilter);
  UNARY("FFT", FFTPadImageFilter);

  MORPH("MathematicalMorphology", GrayscaleDilateImageFilter);
  MORPH("MathematicalMorphology", GrayscaleErodeImageFilter);
  MORPH("MathematicalMorphology", GrayscaleMorphologicalOpeningImageFilter);
  MORPH("MathematicalMorphology", GrayscaleMorphologicalClosingImageFilter);
  MORPH("MathematicalMorphology", MorphologicalGradientImageFilter);
  MORPH("MathematicalMorphology", BlackTopHatImageFilter);
  MORPH("MathematicalMorphology", WhiteTopHatImageFilter);

  Bench(
    "Smoothing",
    "MeanImageFilter",
    [&]() {
      using F = itk::MeanImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::MeanImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Smoothing",
    "MedianImageFilter",
    [&]() {
      using F = itk::MedianImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::MedianImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Smoothing",
    "BoxMeanImageFilter",
    [&]() {
      using F = itk::BoxMeanImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::BoxMeanImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Smoothing",
    "BoxSigmaImageFilter",
    [&]() {
      using F = itk::BoxSigmaImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::BoxSigmaImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetRadius(radius);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Smoothing",
    "DiscreteGaussianImageFilter",
    [&]() {
      using F = itk::DiscreteGaussianImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetVariance(4.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::DiscreteGaussianImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetVariance(4.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Smoothing",
    "SmoothingRecursiveGaussianImageFilter",
    [&]() {
      using F = itk::SmoothingRecursiveGaussianImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetSigma(2.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::SmoothingRecursiveGaussianImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetSigma(2.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "ImageFeature",
    "LaplacianRecursiveGaussianImageFilter",
    [&]() {
      using F = itk::LaplacianRecursiveGaussianImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetSigma(1.5);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::LaplacianRecursiveGaussianImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetSigma(1.5);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "ImageIntensity",
    "RescaleIntensityImageFilter",
    [&]() {
      using F = itk::RescaleIntensityImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetOutputMinimum(0.0f);
      f->SetOutputMaximum(1.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::RescaleIntensityImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetOutputMinimum(0.0);
      f->SetOutputMaximum(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "ImageIntensity",
    "ShiftScaleImageFilter",
    [&]() {
      using F = itk::ShiftScaleImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetShift(0.0);
      f->SetScale(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::ShiftScaleImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetShift(0.0);
      f->SetScale(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Thresholding",
    "BinaryThresholdImageFilter",
    [&]() {
      using F = itk::BinaryThresholdImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetLowerThreshold(50.0f);
      f->SetUpperThreshold(200.0f);
      f->SetInsideValue(1.0f);
      f->SetOutsideValue(0.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::BinaryThresholdImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetLowerThreshold(50.0);
      f->SetUpperThreshold(200.0);
      f->SetInsideValue(1.0);
      f->SetOutsideValue(0.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "Thresholding",
    "OtsuThresholdImageFilter",
    [&]() {
      using F = itk::OtsuThresholdImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetInsideValue(1.0f);
      f->SetOutsideValue(0.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::OtsuThresholdImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetInsideValue(1.0);
      f->SetOutsideValue(0.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);

  BINARY_SELF("ImageCompare", AbsoluteValueDifferenceImageFilter);
  BINARY_SELF("ImageCompare", SquaredDifferenceImageFilter);

  Bench(
    "BinaryMathematicalMorphology",
    "BinaryDilateImageFilter",
    [&]() {
      using F = itk::BinaryDilateImageFilter<FImg, FImg, BallF>;
      auto f = F::New();
      f->SetInput(binF);
      f->SetKernel(kf);
      f->SetForegroundValue(1.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using F = itk::BinaryDilateImageFilter<DImg, DImg, BallD>;
      auto f = F::New();
      f->SetInput(binD);
      f->SetKernel(kd);
      f->SetForegroundValue(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "BinaryMathematicalMorphology",
    "BinaryErodeImageFilter",
    [&]() {
      using F = itk::BinaryErodeImageFilter<FImg, FImg, BallF>;
      auto f = F::New();
      f->SetInput(binF);
      f->SetKernel(kf);
      f->SetForegroundValue(1.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using F = itk::BinaryErodeImageFilter<DImg, DImg, BallD>;
      auto f = F::New();
      f->SetInput(binD);
      f->SetKernel(kd);
      f->SetForegroundValue(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "BinaryMathematicalMorphology",
    "BinaryMorphologicalOpeningImageFilter",
    [&]() {
      using F = itk::BinaryMorphologicalOpeningImageFilter<FImg, FImg, BallF>;
      auto f = F::New();
      f->SetInput(binF);
      f->SetKernel(kf);
      f->SetForegroundValue(1.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using F = itk::BinaryMorphologicalOpeningImageFilter<DImg, DImg, BallD>;
      auto f = F::New();
      f->SetInput(binD);
      f->SetKernel(kd);
      f->SetForegroundValue(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "BinaryMathematicalMorphology",
    "BinaryMorphologicalClosingImageFilter",
    [&]() {
      using F = itk::BinaryMorphologicalClosingImageFilter<FImg, FImg, BallF>;
      auto f = F::New();
      f->SetInput(binF);
      f->SetKernel(kf);
      f->SetForegroundValue(1.0f);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using F = itk::BinaryMorphologicalClosingImageFilter<DImg, DImg, BallD>;
      auto f = F::New();
      f->SetInput(binD);
      f->SetKernel(kd);
      f->SetForegroundValue(1.0);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "DistanceMap",
    "SignedMaurerDistanceMapImageFilter",
    [&]() {
      using F = itk::SignedMaurerDistanceMapImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(binF);
      f->SetInsideIsPositive(true);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using F = itk::SignedMaurerDistanceMapImageFilter<DImg, DImg>;
      auto f = F::New();
      f->SetInput(binD);
      f->SetInsideIsPositive(true);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "DistanceMap",
    "DanielssonDistanceMapImageFilter",
    [&]() {
      using F = itk::DanielssonDistanceMapImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(binF);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using F = itk::DanielssonDistanceMapImageFilter<DImg, DImg>;
      auto f = F::New();
      f->SetInput(binD);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
  Bench(
    "DistanceMap",
    "SignedDanielssonDistanceMapImageFilter",
    [&]() {
      using F = itk::SignedDanielssonDistanceMapImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(binF);
      f->Update();
      return Hold(f->GetOutput());
    },
    [&]() {
      using F = itk::SignedDanielssonDistanceMapImageFilter<DImg, DImg>;
      auto f = F::New();
      f->SetInput(binD);
      f->Update();
      return Hold(f->GetOutput());
    },
    runs);
#include "precision_fill_extra.inc"
#include "precision_fill_construct2.inc"
#include "precision_fill_construct3.inc"
  return 0;
}
