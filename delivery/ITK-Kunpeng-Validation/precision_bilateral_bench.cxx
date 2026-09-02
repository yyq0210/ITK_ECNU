// precision_bilateral_bench — BilateralImageFilter: float vs double 精度 + 速度
// 用法: precision_bilateral_bench <image.png> [domainSigma=4] [rangeSigma=50] [runs=5]
#include "itkBilateralImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkPNGImageIOFactory.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
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

template <unsigned int Dim>
static void
DiffStats(const DImg<Dim> * ref, const FImg<Dim> * test, double & maxAbs, double & rmse, double & maxRel)
{
  maxAbs = 0.0;
  maxRel = 0.0;
  double sumSq = 0.0;
  unsigned long n = 0;
  itk::ImageRegionConstIterator<DImg<Dim>> itR(ref, ref->GetBufferedRegion());
  itk::ImageRegionConstIterator<FImg<Dim>> itT(test, test->GetBufferedRegion());
  for (itR.GoToBegin(), itT.GoToBegin(); !itR.IsAtEnd(); ++itR, ++itT)
  {
    const double vr = itR.Get();
    const double vt = static_cast<double>(itT.Get());
    const double d = std::fabs(vr - vt);
    sumSq += d * d;
    maxAbs = std::max(maxAbs, d);
    maxRel = std::max(maxRel, d / std::max(std::fabs(vr), 1e-12));
    ++n;
  }
  rmse = n > 0 ? std::sqrt(sumSq / static_cast<double>(n)) : 0.0;
}

template <unsigned int Dim>
static void
RunBench(typename FImg<Dim>::Pointer input, double domainSigma, double rangeSigma, int runs)
{
  const auto sz = input->GetLargestPossibleRegion().GetSize();
  std::cout << "\n=== Bilateral dim=" << Dim << " size=";
  for (unsigned int d = 0; d < Dim; ++d)
  {
    if (d)
      std::cout << 'x';
    std::cout << sz[d];
  }
  std::cout << " domainSigma=" << domainSigma << " rangeSigma=" << rangeSigma << " runs=" << runs
            << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << " ===\n";

  auto runFloat = [&]() -> typename FImg<Dim>::Pointer {
    using Bil = itk::BilateralImageFilter<FImg<Dim>, FImg<Dim>>;
    auto b = Bil::New();
    b->SetInput(input);
    b->SetDomainSigma(domainSigma);
    b->SetRangeSigma(rangeSigma);
    b->Update();
    return b->GetOutput();
  };

  auto runDouble = [&]() -> typename DImg<Dim>::Pointer {
    using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
    using Bil = itk::BilateralImageFilter<DImg<Dim>, DImg<Dim>>;
    auto c = Cast::New();
    c->SetInput(input);
    auto b = Bil::New();
    b->SetInput(c->GetOutput());
    b->SetDomainSigma(domainSigma);
    b->SetRangeSigma(rangeSigma);
    b->Update();
    return b->GetOutput();
  };

  auto timeRuns = [&](auto fn) {
    double t = 0.0;
    for (int r = 0; r < runs; ++r)
    {
      const auto t0 = Clock::now();
      fn();
      t += std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
    }
    return t / runs;
  };

  runFloat();
  const double msF = timeRuns(runFloat);
  const double msD = timeRuns(runDouble);

  typename FImg<Dim>::Pointer oF = runFloat();
  typename DImg<Dim>::Pointer oD = runDouble();
  double maxAbs = 0, rmse = 0, maxRel = 0;
  DiffStats<Dim>(oD.GetPointer(), oF.GetPointer(), maxAbs, rmse, maxRel);

  std::cout << std::setprecision(6);
  std::cout << "wall_ms: float=" << msF << " double=" << msD << " speedup=" << (msD / msF) << "x\n";
  std::cout << "vs_double: max_abs=" << maxAbs << " rmse=" << rmse << " max_rel=" << maxRel << '\n';
}

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " <image.png> [domainSigma=4] [rangeSigma=50] [runs=5]\n";
    return 1;
  }
  RegisterIO();
  const std::string path = argv[1];
  const double    domainSigma = argc >= 3 ? std::stod(argv[2]) : 4.0;
  const double    rangeSigma = argc >= 4 ? std::stod(argv[3]) : 50.0;
  const int       runs = argc >= 5 ? std::stoi(argv[4]) : 5;

  try
  {
    using Reader = itk::ImageFileReader<FImg<2>>;
    auto r = Reader::New();
    r->SetFileName(path);
    r->Update();
    RunBench<2>(r->GetOutput(), domainSigma, rangeSigma, runs);
    return 0;
  }
  catch (const std::exception & e)
  {
    std::cerr << "failed: " << e.what() << '\n';
    return 1;
  }
}
