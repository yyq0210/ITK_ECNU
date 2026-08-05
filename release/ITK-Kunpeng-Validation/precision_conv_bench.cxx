// precision_conv_bench — 卷积类重算子：float vs double 精度 + 速度
// 用法: precision_conv_bench <image.png> [runs=5]
#include "itkBilateralImageFilter.h"
#include "itkBoxMeanImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkConvolutionImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFFTConvolutionImageFilter.h"
#include "itkGaussianImageSource.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkMeanImageFilter.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkPNGImageIOFactory.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;

static void
RegisterIO()
{
  itk::PNGImageIOFactory::RegisterOneFactory();
  itk::MetaImageIOFactory::RegisterOneFactory();
}

template <unsigned int Dim>
using FImg = itk::Image<float, Dim>;
template <unsigned int Dim>
using DImg = itk::Image<double, Dim>;

struct DiffResult
{
  double maxAbs{ 0.0 };
  double rmse{ 0.0 };
  double maxRel{ 0.0 };
};

template <unsigned int Dim>
static DiffResult
DiffFD(const DImg<Dim> * ref, const FImg<Dim> * test)
{
  DiffResult r;
  double     sumSq = 0.0;
  unsigned long n = 0;
  itk::ImageRegionConstIterator<DImg<Dim>> itR(ref, ref->GetBufferedRegion());
  itk::ImageRegionConstIterator<FImg<Dim>> itT(test, test->GetBufferedRegion());
  for (itR.GoToBegin(), itT.GoToBegin(); !itR.IsAtEnd(); ++itR, ++itT)
  {
    const double vr = itR.Get();
    const double vt = static_cast<double>(itT.Get());
    const double d = std::fabs(vr - vt);
    sumSq += d * d;
    r.maxAbs = std::max(r.maxAbs, d);
    r.maxRel = std::max(r.maxRel, d / std::max(std::fabs(vr), 1e-12));
    ++n;
  }
  r.rmse = n > 0 ? std::sqrt(sumSq / static_cast<double>(n)) : 0.0;
  return r;
}

template <unsigned int Dim, typename Fn>
static double
TimeRuns(Fn fn, int runs)
{
  double t = 0.0;
  for (int r = 0; r < runs; ++r)
  {
    const auto t0 = Clock::now();
    fn();
    t += std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  }
  return t / runs;
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

template <unsigned int Dim>
static void
Report(const std::string & name, double msF, double msD, const DiffResult & diff)
{
  std::cout << std::fixed << std::setprecision(4);
  std::cout << name << " | ms_float=" << msF << " ms_double=" << msD << " speedup=" << (msD / msF)
            << "x | max_abs=" << std::setprecision(6) << diff.maxAbs << " rmse=" << diff.rmse
            << " max_rel=" << diff.maxRel << '\n';
}

template <unsigned int Dim>
static void
BenchCase(const std::string &                         name,
          const std::function<typename FImg<Dim>::Pointer()> & runF,
          const std::function<typename DImg<Dim>::Pointer()> & runD,
          int                                         runs)
{
  runF();
  const double msF = TimeRuns<Dim>(runF, runs);
  const double msD = TimeRuns<Dim>(runD, runs);
  Report<Dim>(name, msF, msD, DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
}

template <unsigned int Dim>
static void
RunAll(typename FImg<Dim>::Pointer input, int runs)
{
  const auto sz = input->GetLargestPossibleRegion().GetSize();
  std::cout << "\n=== precision_conv_bench dim=" << Dim << " size=";
  for (unsigned int d = 0; d < Dim; ++d)
  {
    if (d)
      std::cout << 'x';
    std::cout << sz[d];
  }
  std::cout << " runs=" << runs << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads()
            << " ===\n";

  const unsigned int pixels = sz[0] * sz[1];
  const bool         large = pixels >= 1024 * 1024;

  // 1) Bilateral（非线性卷积，最重之一）
  BenchCase<Dim>(
    "Bilateral(domain=4,range=50)",
    [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::BilateralImageFilter<FImg<Dim>, FImg<Dim>>;
      auto b = F::New();
      b->SetInput(input);
      b->SetDomainSigma(4.0);
      b->SetRangeSigma(50.0);
      b->Update();
      return b->GetOutput();
    },
    [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::BilateralImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto b = F::New();
      b->SetInput(c->GetOutput());
      b->SetDomainSigma(4.0);
      b->SetRangeSigma(50.0);
      b->Update();
      return b->GetOutput();
    },
    runs);

  // 2) DiscreteGaussian（可分离离散高斯卷积）
  BenchCase<Dim>(
    "DiscreteGaussian(sigma=4)",
    [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::DiscreteGaussianImageFilter<FImg<Dim>, FImg<Dim>>;
      auto g = F::New();
      g->SetInput(input);
      g->SetVariance(16.0);
      g->SetMaximumError(0.01);
      g->Update();
      return g->GetOutput();
    },
    [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::DiscreteGaussianImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto g = F::New();
      g->SetInput(c->GetOutput());
      g->SetVariance(16.0);
      g->SetMaximumError(0.01);
      g->Update();
      return g->GetOutput();
    },
    runs);

  // 3) Mean（盒式卷积，邻域求和）
  {
    typename FImg<Dim>::SizeType radius;
    radius.Fill(15); // 31x31 邻域
    BenchCase<Dim>(
      "Mean(radius=15)",
      [&]() -> typename FImg<Dim>::Pointer {
        using F = itk::MeanImageFilter<FImg<Dim>, FImg<Dim>>;
        auto m = F::New();
        m->SetInput(input);
        m->SetRadius(radius);
        m->Update();
        return m->GetOutput();
      },
      [&]() -> typename DImg<Dim>::Pointer {
        using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
        using F = itk::MeanImageFilter<DImg<Dim>, DImg<Dim>>;
        auto c = Cast::New();
        c->SetInput(input);
        auto m = F::New();
        m->SetInput(c->GetOutput());
        m->SetRadius(radius);
        m->Update();
        return m->GetOutput();
      },
      runs);
  }

  // 3b) BoxMean（itkBoxUtilities AccPixType）
  {
    typename FImg<Dim>::SizeType radius;
    radius.Fill(15);
    BenchCase<Dim>(
      "BoxMean(radius=15)",
      [&]() -> typename FImg<Dim>::Pointer {
        using F = itk::BoxMeanImageFilter<FImg<Dim>, FImg<Dim>>;
        auto m = F::New();
        m->SetInput(input);
        m->SetRadius(radius);
        m->Update();
        return m->GetOutput();
      },
      [&]() -> typename DImg<Dim>::Pointer {
        using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
        using F = itk::BoxMeanImageFilter<DImg<Dim>, DImg<Dim>>;
        auto c = Cast::New();
        c->SetInput(input);
        auto m = F::New();
        m->SetInput(c->GetOutput());
        m->SetRadius(radius);
        m->Update();
        return m->GetOutput();
      },
      runs);
  }

  // 4) 空间域显式卷积（大核，大图时极慢 — 小图或单次 run）
  if (!large)
  {
    auto kernelF = MakeGaussianKernel<FImg<Dim>>(31, 4.0);
    auto kernelD = MakeGaussianKernel<DImg<Dim>>(31, 4.0);
    const int spatialRuns = runs;
    BenchCase<Dim>(
      "ConvolutionSpatial(kernel=31,sigma=4)",
      [&]() -> typename FImg<Dim>::Pointer {
        using F = itk::ConvolutionImageFilter<FImg<Dim>, FImg<Dim>, FImg<Dim>>;
        auto conv = F::New();
        conv->SetInput(input);
        conv->SetKernelImage(kernelF);
        conv->NormalizeOn();
        conv->Update();
        return conv->GetOutput();
      },
      [&]() -> typename DImg<Dim>::Pointer {
        using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
        using F = itk::ConvolutionImageFilter<DImg<Dim>, DImg<Dim>, DImg<Dim>>;
        auto c = Cast::New();
        c->SetInput(input);
        auto conv = F::New();
        conv->SetInput(c->GetOutput());
        conv->SetKernelImage(kernelD);
        conv->NormalizeOn();
        conv->Update();
        return conv->GetOutput();
      },
      spatialRuns);
  }
  else
  {
    std::cout << "ConvolutionSpatial(kernel=31) | skipped (image >= 1024^2, use FFT)\n";
  }

  // 5) FFT 卷积（频域，大核/大图）
  {
    auto kernelF = MakeGaussianKernel<FImg<Dim>>(31, 4.0);
    auto kernelD = MakeGaussianKernel<DImg<Dim>>(31, 4.0);
    BenchCase<Dim>(
      "FFTConvolution(kernel=31,sigma=4)",
      [&]() -> typename FImg<Dim>::Pointer {
        using F = itk::FFTConvolutionImageFilter<FImg<Dim>, FImg<Dim>, FImg<Dim>>;
        auto conv = F::New();
        conv->SetInput(input);
        conv->SetKernelImage(kernelF);
        conv->NormalizeOn();
        conv->Update();
        return conv->GetOutput();
      },
      [&]() -> typename DImg<Dim>::Pointer {
        using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
        using F = itk::FFTConvolutionImageFilter<DImg<Dim>, DImg<Dim>, DImg<Dim>>;
        auto c = Cast::New();
        c->SetInput(input);
        auto conv = F::New();
        conv->SetInput(c->GetOutput());
        conv->SetKernelImage(kernelD);
        conv->NormalizeOn();
        conv->Update();
        return conv->GetOutput();
      },
      runs);
  }
}

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " <image.png> [runs=5]\n";
    return 1;
  }
  RegisterIO();
  const std::string path = argv[1];
  const int         runs = argc >= 3 ? std::stoi(argv[2]) : 5;

  try
  {
    using Reader = itk::ImageFileReader<FImg<2>>;
    auto r = Reader::New();
    r->SetFileName(path);
    r->Update();
    RunAll<2>(r->GetOutput(), runs);
    return 0;
  }
  catch (const std::exception & e)
  {
    std::cerr << "failed: " << e.what() << '\n';
    return 1;
  }
}
