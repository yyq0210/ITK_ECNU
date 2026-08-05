// hybrid_precision_real_bench — 真实 ITK 图像：full-float / hybrid / full-double 高斯平滑
// 用法: hybrid_precision_real_bench <file.mha|png> [sigma] [runs]
#include "itkCastImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkObjectFactoryBase.h"
#include "itkPNGImageIOFactory.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"

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
static typename FImg<Dim>::Pointer
PadIfNeeded(typename FImg<Dim>::Pointer img)
{
  auto                     sz = img->GetLargestPossibleRegion().GetSize();
  typename FImg<Dim>::SizeType lower, upper;
  lower.Fill(0);
  upper.Fill(0);
  bool need = false;
  for (unsigned int d = 0; d < Dim; ++d)
  {
    if (sz[d] < 4)
    {
      upper[d] = 4 - sz[d];
      need = true;
    }
  }
  if (!need)
    return img;
  using Pad = itk::ConstantPadImageFilter<FImg<Dim>, FImg<Dim>>;
  auto p = Pad::New();
  p->SetInput(img);
  p->SetPadLowerBound(lower);
  p->SetPadUpperBound(upper);
  p->SetConstant(0.0f);
  p->Update();
  return p->GetOutput();
}

template <unsigned int Dim>
static double
DiffStatsF(const DImg<Dim> * ref, const FImg<Dim> * test, double & maxAbs, double & maxRel)
{
  maxAbs = 0.0;
  maxRel = 0.0;
  double sumSq = 0.0;
  unsigned long n = 0;
  itk::ImageRegionConstIterator<DImg<Dim>> itR(ref, ref->GetBufferedRegion());
  itk::ImageRegionConstIterator<FImg<Dim>>  itT(test, test->GetBufferedRegion());
  for (itR.GoToBegin(), itT.GoToBegin(); !itR.IsAtEnd(); ++itR, ++itT)
  {
    const double vr = itR.Get();
    const double vt = static_cast<double>(itT.Get());
    const double d = std::fabs(vr - vt);
    sumSq += d * d;
    if (d > maxAbs)
      maxAbs = d;
    const double denom = std::max(std::fabs(vr), 1e-12);
    const double rel = d / denom;
    if (rel > maxRel)
      maxRel = rel;
    ++n;
  }
  const double rmse = n > 0 ? std::sqrt(sumSq / static_cast<double>(n)) : 0.0;
  return rmse;
}

template <unsigned int Dim>
static void
RunBench(typename FImg<Dim>::Pointer input, double sigma, int runs, const std::string & label)
{
  input = PadIfNeeded<Dim>(input);
  const auto sz = input->GetLargestPossibleRegion().GetSize();
  std::cout << "\n=== " << label << " dim=" << Dim << " size=";
  for (unsigned int d = 0; d < Dim; ++d)
  {
    if (d)
      std::cout << 'x';
    std::cout << sz[d];
  }
  std::cout << " sigma=" << sigma << " runs=" << runs << '\n';

  auto runFloat = [&]() -> typename FImg<Dim>::Pointer {
    using SmoothF = itk::SmoothingRecursiveGaussianImageFilter<FImg<Dim>, FImg<Dim>>;
    auto s = SmoothF::New();
    s->SetInput(input);
    s->SetSigma(sigma);
    s->Update();
    return s->GetOutput();
  };

  auto runHybrid = [&]() -> typename FImg<Dim>::Pointer {
    using CastUp = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
    using SmoothD = itk::SmoothingRecursiveGaussianImageFilter<DImg<Dim>, DImg<Dim>>;
    using CastDown = itk::CastImageFilter<DImg<Dim>, FImg<Dim>>;
    auto up = CastUp::New();
    up->SetInput(input);
    auto s = SmoothD::New();
    s->SetInput(up->GetOutput());
    s->SetSigma(sigma);
    auto down = CastDown::New();
    down->SetInput(s->GetOutput());
    down->Update();
    return down->GetOutput();
  };

  auto runDouble = [&]() -> typename DImg<Dim>::Pointer {
    using CastUp = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
    using SmoothD = itk::SmoothingRecursiveGaussianImageFilter<DImg<Dim>, DImg<Dim>>;
    auto up = CastUp::New();
    up->SetInput(input);
    auto s = SmoothD::New();
    s->SetInput(up->GetOutput());
    s->SetSigma(sigma);
    s->Update();
    return s->GetOutput();
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
  const double msH = timeRuns(runHybrid);
  const double msD = timeRuns(runDouble);

  auto oF = runFloat();
  auto oH = runHybrid();
  auto oD = runDouble();

  double maxF = 0, maxH = 0, relF = 0, relH = 0;
  const double rmseF = DiffStatsF<Dim>(oD.GetPointer(), oF.GetPointer(), maxF, relF);
  const double rmseH = DiffStatsF<Dim>(oD.GetPointer(), oH.GetPointer(), maxH, relH);

  std::cout << std::setprecision(8);
  std::cout << "wall_ms: float=" << msF << " hybrid=" << msH << " double=" << msD << '\n';
  std::cout << "speedup_vs_double: float=" << (msD / msF) << "x hybrid=" << (msD / msH) << "x\n";
  std::cout << "vs_double ref — float:  max_abs=" << maxF << " rmse=" << rmseF << " max_rel=" << relF << '\n';
  std::cout << "vs_double ref — hybrid: max_abs=" << maxH << " rmse=" << rmseH << " max_rel=" << relH << '\n';
}

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " <image.mha|.png> [sigma=2.0] [runs=5]\n";
    return 1;
  }
  RegisterIO();
  const std::string path = argv[1];
  const double    sigma = argc >= 3 ? std::stod(argv[2]) : 2.0;
  const int       runs = argc >= 4 ? std::stoi(argv[3]) : 5;

  std::cout << "file=" << path << " ITK_threads="
            << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << '\n';

  // 先读维度
  using Image2D = FImg<2>;
  using Image3D = FImg<3>;
  using Reader2 = itk::ImageFileReader<Image2D>;
  using Reader3 = itk::ImageFileReader<Image3D>;

  try
  {
    auto r2 = Reader2::New();
    r2->SetFileName(path);
    r2->UpdateOutputInformation();
    if (r2->GetImageIO()->GetNumberOfComponents() > 0 && r2->GetImageIO()->GetNumberOfDimensions() <= 2)
    {
      r2->Update();
      RunBench<2>(r2->GetOutput(), sigma, runs, path);
      return 0;
    }
  }
  catch (...)
  {
  }

  try
  {
    auto r3 = Reader3::New();
    r3->SetFileName(path);
    r3->Update();
    RunBench<3>(r3->GetOutput(), sigma, runs, path);
    return 0;
  }
  catch (const std::exception & e)
  {
    std::cerr << "read/run failed: " << e.what() << '\n';
    return 1;
  }
}
