// precision_diffusion_bench — 扫频 TopN：CAD/GAD float vs double
// 用法: precision_diffusion_bench <image.png> [iterations=50] [timeStep=0.125] [conductance=3] [runs=5]
#include "itkCastImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
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
  DiffResult    r;
  double        sumSq = 0.0;
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
  runD();
  const double msF = TimeRuns<Dim>(runF, runs);
  const double msD = TimeRuns<Dim>(runD, runs);
  Report<Dim>(name, msF, msD, DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
}

template <unsigned int Dim>
static void
RunAll(typename FImg<Dim>::Pointer input, int iterations, double conductance, double timeStep, int runs)
{
  const auto sz = input->GetLargestPossibleRegion().GetSize();
  std::cout << "\n=== precision_diffusion_bench dim=" << Dim << " size=";
  for (unsigned int d = 0; d < Dim; ++d)
  {
    if (d)
      std::cout << 'x';
    std::cout << sz[d];
  }
  std::cout << " iter=" << iterations << " conductance=" << conductance << " timeStep=" << timeStep
            << " runs=" << runs << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads()
            << " ===\n";

  BenchCase<Dim>(
    "CAD(iter=" + std::to_string(iterations) + ")",
    [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::CurvatureAnisotropicDiffusionImageFilter<FImg<Dim>, FImg<Dim>>;
      auto f = F::New();
      f->SetInput(input);
      f->SetNumberOfIterations(iterations);
      f->SetConductanceParameter(conductance);
      f->SetTimeStep(timeStep);
      f->Update();
      return f->GetOutput();
    },
    [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::CurvatureAnisotropicDiffusionImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetNumberOfIterations(iterations);
      f->SetConductanceParameter(conductance);
      f->SetTimeStep(timeStep);
      f->Update();
      return f->GetOutput();
    },
    runs);

  BenchCase<Dim>(
    "GAD(iter=" + std::to_string(iterations) + ")",
    [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::GradientAnisotropicDiffusionImageFilter<FImg<Dim>, FImg<Dim>>;
      auto f = F::New();
      f->SetInput(input);
      f->SetNumberOfIterations(iterations);
      f->SetConductanceParameter(conductance);
      f->SetTimeStep(timeStep);
      f->Update();
      return f->GetOutput();
    },
    [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::GradientAnisotropicDiffusionImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetNumberOfIterations(iterations);
      f->SetConductanceParameter(conductance);
      f->SetTimeStep(timeStep);
      f->Update();
      return f->GetOutput();
    },
    runs);
}

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0]
              << " <image.png> [iterations=50] [timeStep=0.125] [conductance=3] [runs=5]\n";
    return 1;
  }
  RegisterIO();
  const std::string path = argv[1];
  const int         iterations = argc >= 3 ? std::stoi(argv[2]) : 50;
  const double      timeStep = argc >= 4 ? std::stod(argv[3]) : 0.125;
  const double      conductance = argc >= 5 ? std::stod(argv[4]) : 3.0;
  const int         runs = argc >= 6 ? std::stoi(argv[5]) : 5;

  try
  {
    using Reader = itk::ImageFileReader<FImg<2>>;
    auto r = Reader::New();
    r->SetFileName(path);
    r->Update();
    RunAll<2>(r->GetOutput(), iterations, conductance, timeStep, runs);
    return 0;
  }
  catch (const std::exception & e)
  {
    std::cerr << "failed: " << e.what() << '\n';
    return 1;
  }
}
