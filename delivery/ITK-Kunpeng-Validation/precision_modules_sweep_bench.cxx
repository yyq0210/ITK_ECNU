// precision_modules_sweep_bench — 各 Filtering/Segmentation 代表算子：full_float vs full_double
// 用法: precision_modules_sweep_bench <image.png> [runs=3]
// 输出 CSV：module,operator,ms_float,ms_double,speedup,max_abs,rmse
#include "itkAbsoluteValueDifferenceImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBoxMeanImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkConvolutionImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFFTConvolutionImageFilter.h"
#include "itkGaussianImageSource.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkGrayscaleDilateImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkLaplacianRecursiveGaussianImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMeanImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkMetaImageIOFactory.h"
#include "itkMinMaxCurvatureFlowImageFilter.h"
#include "itkMultiThreaderBase.h"
#include "itkNormalizeImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkPNGImageIOFactory.h"
#include "itkResampleImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkSobelEdgeDetectionImageFilter.h"

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
  double     sumSq = 0.0;
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
Emit(const std::string & module,
     const std::string & op,
     double              msF,
     double              msD,
     const DiffResult &  d)
{
  const double sp = msF > 0.0 ? (msD / msF) : 0.0;
  std::cout << std::fixed << std::setprecision(4);
  std::cout << module << ',' << op << ',' << msF << ',' << msD << ',' << sp << ',' << std::setprecision(8)
            << d.maxAbs << ',' << d.rmse << '\n';
}

static void
Bench(const std::string &                 module,
      const std::string &                 op,
      const std::function<FImg::Pointer()> & runF,
      const std::function<DImg::Pointer()> & runD,
      int                                 runs)
{
  std::cerr << ">> " << module << '/' << op << std::endl;
  try
  {
    runF();
    const double    msF = TimeRuns(runF, runs);
    const double    msD = TimeRuns(runD, runs);
    const DiffResult d = DiffFD(runD().GetPointer(), runF().GetPointer());
    Emit(module, op, msF, msD, d);
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
MakeGaussianKernel(unsigned int kernelSize, double sigma)
{
  using Source = itk::GaussianImageSource<TImage>;
  auto src = Source::New();
  typename TImage::SizeType size;
  size.Fill(kernelSize);
  typename TImage::PointType mean;
  mean.Fill(static_cast<double>(kernelSize) / 2.0 - 0.5);
  src->SetSize(size);
  src->SetMean(mean);
  src->SetSigma(sigma);
  src->SetNormalized(true);
  src->Update();
  return src->GetOutput();
}

static FImg::Pointer
MakeBinary(FImg * input)
{
  using Otsu = itk::OtsuThresholdImageFilter<FImg, FImg>;
  auto o = Otsu::New();
  o->SetInput(input);
  o->SetInsideValue(1.0f);
  o->SetOutsideValue(0.0f);
  o->Update();
  return o->GetOutput();
}

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " <image.png> [runs=3]\n";
    return 1;
  }
  RegisterIO();
  const int runs = argc >= 3 ? std::stoi(argv[2]) : 3;

  using Reader = itk::ImageFileReader<FImg>;
  auto reader = Reader::New();
  reader->SetFileName(argv[1]);
  reader->Update();
  FImg::Pointer input = reader->GetOutput();
  const auto    sz = input->GetLargestPossibleRegion().GetSize();

  std::cerr << "loaded " << sz[0] << 'x' << sz[1] << " runs=" << runs << std::endl;
  std::cout << "# precision_modules_sweep_bench size=" << sz[0] << 'x' << sz[1]
            << " runs=" << runs
            << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << '\n';
  std::cout << "module,operator,ms_float,ms_double,speedup,max_abs,rmse\n";
  std::cout.flush();

  Bench(
    "Smoothing",
    "RecursiveGaussian",
    [&]() {
      using F = itk::SmoothingRecursiveGaussianImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetSigma(2.0);
      f->Update();
      return f->GetOutput();
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
      return f->GetOutput();
    },
    runs);

  Bench(
    "Smoothing",
    "DiscreteGaussian",
    [&]() {
      using F = itk::DiscreteGaussianImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetVariance(4.0);
      f->SetMaximumError(0.01);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::DiscreteGaussianImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetVariance(4.0);
      f->SetMaximumError(0.01);
      f->Update();
      return f->GetOutput();
    },
    runs);

  {
    FImg::SizeType radius;
    radius.Fill(2);
    Bench(
      "Smoothing",
      "Mean",
      [&]() {
        using F = itk::MeanImageFilter<FImg, FImg>;
        auto f = F::New();
        f->SetInput(input);
        f->SetRadius(radius);
        f->Update();
        return f->GetOutput();
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
        return f->GetOutput();
      },
      runs);
    Bench(
      "Smoothing",
      "BoxMean",
      [&]() {
        using F = itk::BoxMeanImageFilter<FImg, FImg>;
        auto f = F::New();
        f->SetInput(input);
        f->SetRadius(radius);
        f->Update();
        return f->GetOutput();
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
        return f->GetOutput();
      },
      runs);
    Bench(
      "Smoothing",
      "Median",
      [&]() {
        using F = itk::MedianImageFilter<FImg, FImg>;
        auto f = F::New();
        f->SetInput(input);
        f->SetRadius(radius);
        f->Update();
        return f->GetOutput();
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
        return f->GetOutput();
      },
      runs);
  }

  Bench(
    "ImageFeature",
    "SobelEdge",
    [&]() {
      using F = itk::SobelEdgeDetectionImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::SobelEdgeDetectionImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "ImageFeature",
    "LaplacianRecursiveGaussian",
    [&]() {
      using F = itk::LaplacianRecursiveGaussianImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetSigma(1.5);
      f->Update();
      return f->GetOutput();
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
      return f->GetOutput();
    },
    runs);

  Bench(
    "ImageGradient",
    "GradientMagnitude",
    [&]() {
      using F = itk::GradientMagnitudeImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::GradientMagnitudeImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "ImageIntensity",
    "Normalize",
    [&]() {
      using F = itk::NormalizeImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::NormalizeImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "ImageIntensity",
    "RescaleIntensity",
    [&]() {
      using F = itk::RescaleIntensityImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetOutputMinimum(0.0f);
      f->SetOutputMaximum(1.0f);
      f->Update();
      return f->GetOutput();
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
      return f->GetOutput();
    },
    runs);

  Bench(
    "ImageGrid",
    "ResampleIdentity",
    [&]() {
      using Xf = itk::IdentityTransform<double, Dim>;
      using Interp = itk::LinearInterpolateImageFunction<FImg, double>;
      using F = itk::ResampleImageFilter<FImg, FImg, double>;
      auto f = F::New();
      f->SetInput(input);
      f->SetTransform(Xf::New());
      f->SetInterpolator(Interp::New());
      f->SetSize(input->GetLargestPossibleRegion().GetSize());
      f->SetOutputSpacing(input->GetSpacing());
      f->SetOutputOrigin(input->GetOrigin());
      f->SetOutputDirection(input->GetDirection());
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using Xf = itk::IdentityTransform<double, Dim>;
      using Interp = itk::LinearInterpolateImageFunction<DImg, double>;
      using F = itk::ResampleImageFilter<DImg, DImg, double>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetTransform(Xf::New());
      f->SetInterpolator(Interp::New());
      f->SetSize(input->GetLargestPossibleRegion().GetSize());
      f->SetOutputSpacing(input->GetSpacing());
      f->SetOutputOrigin(input->GetOrigin());
      f->SetOutputDirection(input->GetDirection());
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "ImageCompare",
    "AbsoluteDifference_self",
    [&]() {
      using F = itk::AbsoluteValueDifferenceImageFilter<FImg, FImg, FImg>;
      auto f = F::New();
      f->SetInput1(input);
      f->SetInput2(input);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::AbsoluteValueDifferenceImageFilter<DImg, DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      c->Update();
      auto f = F::New();
      f->SetInput1(c->GetOutput());
      f->SetInput2(c->GetOutput());
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "Thresholding",
    "Otsu",
    [&]() {
      using F = itk::OtsuThresholdImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetInsideValue(1.0f);
      f->SetOutsideValue(0.0f);
      f->Update();
      return f->GetOutput();
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
      return f->GetOutput();
    },
    runs);

  Bench(
    "Thresholding",
    "BinaryThreshold",
    [&]() {
      using F = itk::BinaryThresholdImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetLowerThreshold(50.0f);
      f->SetUpperThreshold(200.0f);
      f->SetInsideValue(1.0f);
      f->SetOutsideValue(0.0f);
      f->Update();
      return f->GetOutput();
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
      return f->GetOutput();
    },
    runs);

  {
    using BallF = itk::BinaryBallStructuringElement<float, Dim>;
    using BallD = itk::BinaryBallStructuringElement<double, Dim>;
    BallF kf;
    kf.SetRadius(1);
    kf.CreateStructuringElement();
    BallD kd;
    kd.SetRadius(1);
    kd.CreateStructuringElement();
    Bench(
      "MathematicalMorphology",
      "GrayscaleDilate",
      [&]() {
        using F = itk::GrayscaleDilateImageFilter<FImg, FImg, BallF>;
        auto f = F::New();
        f->SetInput(input);
        f->SetKernel(kf);
        f->Update();
        return f->GetOutput();
      },
      [&]() {
        using Cast = itk::CastImageFilter<FImg, DImg>;
        using F = itk::GrayscaleDilateImageFilter<DImg, DImg, BallD>;
        auto c = Cast::New();
        c->SetInput(input);
        auto f = F::New();
        f->SetInput(c->GetOutput());
        f->SetKernel(kd);
        f->Update();
        return f->GetOutput();
      },
      runs);
  }

  {
    auto binF = MakeBinary(input);
    using Ball = itk::BinaryBallStructuringElement<float, Dim>;
    Ball k;
    k.SetRadius(1);
    k.CreateStructuringElement();
    Bench(
      "BinaryMathematicalMorphology",
      "BinaryDilate",
      [&]() {
        using F = itk::BinaryDilateImageFilter<FImg, FImg, Ball>;
        auto f = F::New();
        f->SetInput(binF);
        f->SetKernel(k);
        f->SetForegroundValue(1.0f);
        f->Update();
        return f->GetOutput();
      },
      [&]() {
        using Cast = itk::CastImageFilter<FImg, DImg>;
        using BallD = itk::BinaryBallStructuringElement<double, Dim>;
        using F = itk::BinaryDilateImageFilter<DImg, DImg, BallD>;
        auto c = Cast::New();
        c->SetInput(binF);
        BallD kd;
        kd.SetRadius(1);
        kd.CreateStructuringElement();
        auto f = F::New();
        f->SetInput(c->GetOutput());
        f->SetKernel(kd);
        f->SetForegroundValue(1.0);
        f->Update();
        return f->GetOutput();
      },
      runs);

    Bench(
      "DistanceMap",
      "SignedMaurer",
      [&]() {
        using F = itk::SignedMaurerDistanceMapImageFilter<FImg, FImg>;
        auto f = F::New();
        f->SetInput(binF);
        f->SetInsideIsPositive(true);
        f->Update();
        return f->GetOutput();
      },
      [&]() {
        using Cast = itk::CastImageFilter<FImg, DImg>;
        using F = itk::SignedMaurerDistanceMapImageFilter<DImg, DImg>;
        auto c = Cast::New();
        c->SetInput(binF);
        auto f = F::New();
        f->SetInput(c->GetOutput());
        f->SetInsideIsPositive(true);
        f->Update();
        return f->GetOutput();
      },
      runs);

  }

  Bench(
    "CurvatureFlow",
    "CurvatureFlow",
    [&]() {
      using F = itk::CurvatureFlowImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetTimeStep(0.125);
      f->SetNumberOfIterations(8);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::CurvatureFlowImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetTimeStep(0.125);
      f->SetNumberOfIterations(8);
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "Denoising",
    "MinMaxCurvatureFlow",
    [&]() {
      using F = itk::MinMaxCurvatureFlowImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetTimeStep(0.125);
      f->SetNumberOfIterations(6);
      f->SetStencilRadius(1);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::MinMaxCurvatureFlowImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetTimeStep(0.125);
      f->SetNumberOfIterations(6);
      f->SetStencilRadius(1);
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "AnisotropicSmoothing",
    "CAD",
    [&]() {
      using F = itk::CurvatureAnisotropicDiffusionImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetNumberOfIterations(20);
      f->SetTimeStep(0.125);
      f->SetConductanceParameter(3.0);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::CurvatureAnisotropicDiffusionImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetNumberOfIterations(20);
      f->SetTimeStep(0.125);
      f->SetConductanceParameter(3.0);
      f->Update();
      return f->GetOutput();
    },
    runs);

  Bench(
    "AnisotropicSmoothing",
    "GAD",
    [&]() {
      using F = itk::GradientAnisotropicDiffusionImageFilter<FImg, FImg>;
      auto f = F::New();
      f->SetInput(input);
      f->SetNumberOfIterations(20);
      f->SetTimeStep(0.125);
      f->SetConductanceParameter(3.0);
      f->Update();
      return f->GetOutput();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      using F = itk::GradientAnisotropicDiffusionImageFilter<DImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetNumberOfIterations(20);
      f->SetTimeStep(0.125);
      f->SetConductanceParameter(3.0);
      f->Update();
      return f->GetOutput();
    },
    runs);

  {
    auto kF = MakeGaussianKernel<FImg>(11, 2.0);
    auto kD = MakeGaussianKernel<DImg>(11, 2.0);
    Bench(
      "Convolution",
      "ConvolutionSpatial",
      [&]() {
        using F = itk::ConvolutionImageFilter<FImg, FImg, FImg>;
        auto f = F::New();
        f->SetInput(input);
        f->SetKernelImage(kF);
        f->NormalizeOn();
        f->Update();
        return f->GetOutput();
      },
      [&]() {
        using Cast = itk::CastImageFilter<FImg, DImg>;
        using F = itk::ConvolutionImageFilter<DImg, DImg, DImg>;
        auto c = Cast::New();
        c->SetInput(input);
        auto f = F::New();
        f->SetInput(c->GetOutput());
        f->SetKernelImage(kD);
        f->NormalizeOn();
        f->Update();
        return f->GetOutput();
      },
      runs);
    Bench(
      "FFT",
      "FFTConvolution",
      [&]() {
        using F = itk::FFTConvolutionImageFilter<FImg, FImg, FImg>;
        auto f = F::New();
        f->SetInput(input);
        f->SetKernelImage(kF);
        f->NormalizeOn();
        f->Update();
        return f->GetOutput();
      },
      [&]() {
        using Cast = itk::CastImageFilter<FImg, DImg>;
        using F = itk::FFTConvolutionImageFilter<DImg, DImg, DImg>;
        auto c = Cast::New();
        c->SetInput(input);
        auto f = F::New();
        f->SetInput(c->GetOutput());
        f->SetKernelImage(kD);
        f->NormalizeOn();
        f->Update();
        return f->GetOutput();
      },
      runs);
  }

  return 0;
}
