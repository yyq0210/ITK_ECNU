// hybrid_precision_bench — 对比 full-float / hybrid / full-double 的墙钟与精度
#include "itkCastImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkPNGImageIOFactory.h"
#include "itkObjectFactoryBase.h"
#include "itkImageRegionIterator.h"
#include "itkMedianImageFilter.h"
#include "itkMultiThreaderBase.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;

template <typename TImage>
static void
FillCheckerboard(TImage * image, float hi = 1000.0F)
{
  const auto region = image->GetLargestPossibleRegion();
  image->SetRegions(region);
  image->Allocate();
  itk::ImageRegionIterator<TImage> it(image, region);
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto         idx = it.GetIndex();
    const unsigned int x = static_cast<unsigned int>(idx[0]);
    const unsigned int y = static_cast<unsigned int>(idx[1]);
    const float        v = ((x / 32) + (y / 32)) % 2 == 0 ? 0.0F : hi;
    it.Set(static_cast<typename TImage::PixelType>(v));
  }
}

static double
DiffStats(const itk::Image<double, 2> * ref, const itk::Image<float, 2> * test, double & maxAbs)
{
  maxAbs = 0.0;
  double sum = 0.0;
  unsigned long n = 0;
  itk::ImageRegionConstIterator<itk::Image<double, 2>> itR(ref, ref->GetBufferedRegion());
  itk::ImageRegionConstIterator<itk::Image<float, 2>>  itT(test, test->GetBufferedRegion());
  for (itR.GoToBegin(), itT.GoToBegin(); !itR.IsAtEnd(); ++itR, ++itT)
  {
    const double d = std::fabs(static_cast<double>(itR.Get()) - static_cast<double>(itT.Get()));
    sum += d;
    if (d > maxAbs)
      maxAbs = d;
    ++n;
  }
  return n > 0 ? sum / static_cast<double>(n) : 0.0;
}

static double
DiffStatsDD(const itk::Image<double, 2> * ref, const itk::Image<double, 2> * test, double & maxAbs)
{
  maxAbs = 0.0;
  double sum = 0.0;
  unsigned long n = 0;
  itk::ImageRegionConstIterator<itk::Image<double, 2>> itR(ref, ref->GetBufferedRegion());
  itk::ImageRegionConstIterator<itk::Image<double, 2>> itT(test, test->GetBufferedRegion());
  for (itR.GoToBegin(), itT.GoToBegin(); !itR.IsAtEnd(); ++itR, ++itT)
  {
    const double d = std::fabs(itR.Get() - itT.Get());
    sum += d;
    if (d > maxAbs)
      maxAbs = d;
    ++n;
  }
  return n > 0 ? sum / static_cast<double>(n) : 0.0;
}

int
main(int argc, char ** argv)
{
  constexpr unsigned int Dim = 2;
  using FloatImage = itk::Image<float, Dim>;
  using DoubleImage = itk::Image<double, Dim>;

  std::string inputPath;
  unsigned int size = 1024;
  double sigma = 2.0;
  int runs = 3;
  if (argc >= 2)
    inputPath = argv[1];
  if (argc >= 3)
    size = static_cast<unsigned int>(std::stoul(argv[2]));
  if (argc >= 4)
    sigma = std::stod(argv[3]);
  if (argc >= 5)
    runs = std::stoi(argv[4]);

  FloatImage::Pointer input = FloatImage::New();
  if (!inputPath.empty())
  {
    itk::PNGImageIOFactory::RegisterOneFactory();
    using Reader = itk::ImageFileReader<FloatImage>;
    auto reader = Reader::New();
    reader->SetFileName(inputPath);
    reader->Update();
    input = reader->GetOutput();
  }
  else
  {
    FloatImage::SizeType   sz;
    FloatImage::IndexType  idx;
    FloatImage::RegionType reg;
    sz.Fill(size);
    idx.Fill(0);
    reg.SetSize(sz);
    reg.SetIndex(idx);
    input->SetRegions(reg);
    FillCheckerboard(input.GetPointer());
  }

  const auto nthreads = itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads();
  std::cout << "=== hybrid_precision_bench ===\n";
  std::cout << "size=" << input->GetLargestPossibleRegion().GetSize()[0] << "x"
            << input->GetLargestPossibleRegion().GetSize()[1] << " sigma=" << sigma
            << " runs=" << runs << " ITK_threads=" << nthreads << '\n';

  auto medianRadius = itk::Size<Dim>();
  medianRadius.Fill(1);

  auto runFloatOnly = [&]() -> FloatImage::Pointer {
    using MedianF = itk::MedianImageFilter<FloatImage, FloatImage>;
    using SmoothF = itk::SmoothingRecursiveGaussianImageFilter<FloatImage, FloatImage>;
    auto m = MedianF::New();
    m->SetRadius(medianRadius);
    m->SetInput(input);
    auto s = SmoothF::New();
    s->SetInput(m->GetOutput());
    s->SetSigma(sigma);
    s->Update();
    return s->GetOutput();
  };

  auto runHybrid = [&]() -> FloatImage::Pointer {
    using MedianF = itk::MedianImageFilter<FloatImage, FloatImage>;
    using CastUp = itk::CastImageFilter<FloatImage, DoubleImage>;
    using SmoothD = itk::SmoothingRecursiveGaussianImageFilter<DoubleImage, DoubleImage>;
    using CastDown = itk::CastImageFilter<DoubleImage, FloatImage>;
    auto m = MedianF::New();
    m->SetRadius(medianRadius);
    m->SetInput(input);
    auto up = CastUp::New();
    up->SetInput(m->GetOutput());
    auto s = SmoothD::New();
    s->SetInput(up->GetOutput());
    s->SetSigma(sigma);
    auto down = CastDown::New();
    down->SetInput(s->GetOutput());
    down->Update();
    return down->GetOutput();
  };

  auto runDoubleOnly = [&]() -> DoubleImage::Pointer {
    using CastUp = itk::CastImageFilter<FloatImage, DoubleImage>;
    using MedianD = itk::MedianImageFilter<DoubleImage, DoubleImage>;
    using SmoothD = itk::SmoothingRecursiveGaussianImageFilter<DoubleImage, DoubleImage>;
    auto up = CastUp::New();
    up->SetInput(input);
    auto m = MedianD::New();
    m->SetRadius(medianRadius);
    m->SetInput(up->GetOutput());
    auto s = SmoothD::New();
    s->SetInput(m->GetOutput());
    s->SetSigma(sigma);
    s->Update();
    return s->GetOutput();
  };

  auto timeRuns = [&](auto fn) {
    double totalMs = 0.0;
    for (int r = 0; r < runs; ++r)
    {
      const auto t0 = Clock::now();
      fn();
      const auto t1 = Clock::now();
      totalMs += std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    return totalMs / static_cast<double>(runs);
  };

  // warmup
  runFloatOnly();

  const double msFloat = timeRuns(runFloatOnly);
  const double msHybrid = timeRuns(runHybrid);
  const double msDouble = timeRuns(runDoubleOnly);

  auto outFloat = runFloatOnly();
  auto outHybrid = runHybrid();
  auto outDouble = runDoubleOnly();

  double maxF = 0, maxH = 0;
  const double maeF = DiffStats(outDouble.GetPointer(), outFloat.GetPointer(), maxF);
  const double maeH = DiffStats(outDouble.GetPointer(), outHybrid.GetPointer(), maxH);

  std::cout << std::setprecision(6);
  std::cout << "--- wall ms (avg of " << runs << ") ---\n";
  std::cout << "full_float:  " << msFloat << " ms\n";
  std::cout << "hybrid:      " << msHybrid << " ms  (float median + double gaussian)\n";
  std::cout << "full_double: " << msDouble << " ms\n";
  std::cout << "--- speedup vs full_double (reference) ---\n";
  std::cout << "full_float:  " << (msDouble / msFloat) << "x faster, precision loss vs double: max_abs="
            << maxF << " mean_abs=" << maeF << '\n';
  std::cout << "hybrid:      " << (msDouble / msHybrid) << "x faster, precision loss vs double: max_abs="
            << maxH << " mean_abs=" << maeH << '\n';
  std::cout << "hybrid vs float speed ratio (float/hybrid): " << (msHybrid / msFloat) << "x time of float-only\n";

  return 0;
}
