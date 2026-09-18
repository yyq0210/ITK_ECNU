// precision_pipeline_bench — 滤波/分割：full_float vs full_double（无 hybrid cast 链）
// 用法: precision_pipeline_bench [image.png|mha] [runs=5]
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkGrayscaleDilateImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkMedianImageFilter.h"
#include "itkNormalizeImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkPNGImageIOFactory.h"
#include "itkResampleImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkSobelEdgeDetectionImageFilter.h"

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

struct DiffResult
{
  double maxAbs{ 0.0 };
  double meanAbs{ 0.0 };
  double rmse{ 0.0 };
  double maxRel{ 0.0 };
};

template <unsigned int Dim>
static DiffResult
DiffFD(const DImg<Dim> * ref, const FImg<Dim> * test)
{
  DiffResult r;
  double     sumAbs = 0.0;
  double     sumSq = 0.0;
  unsigned long n = 0;
  itk::ImageRegionConstIterator<DImg<Dim>> itR(ref, ref->GetBufferedRegion());
  itk::ImageRegionConstIterator<FImg<Dim>> itT(test, test->GetBufferedRegion());
  for (itR.GoToBegin(), itT.GoToBegin(); !itR.IsAtEnd(); ++itR, ++itT)
  {
    const double vr = itR.Get();
    const double vt = static_cast<double>(itT.Get());
    const double d = std::fabs(vr - vt);
    sumAbs += d;
    sumSq += d * d;
    r.maxAbs = std::max(r.maxAbs, d);
    r.maxRel = std::max(r.maxRel, d / std::max(std::fabs(vr), 1e-12));
    ++n;
  }
  if (n > 0)
  {
    r.meanAbs = sumAbs / static_cast<double>(n);
    r.rmse = std::sqrt(sumSq / static_cast<double>(n));
  }
  return r;
}

template <unsigned int Dim>
static typename FImg<Dim>::Pointer
PadMinSize(typename FImg<Dim>::Pointer img, unsigned int minSz = 4)
{
  auto sz = img->GetLargestPossibleRegion().GetSize();
  typename FImg<Dim>::SizeType lower, upper;
  lower.Fill(0);
  upper.Fill(0);
  bool need = false;
  for (unsigned int d = 0; d < Dim; ++d)
  {
    if (sz[d] < minSz)
    {
      upper[d] = minSz - sz[d];
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
ReportCase(const std::string & name,
           double              msF,
           double              msD,
           const DiffResult &  diff)
{
  std::cout << std::fixed << std::setprecision(4);
  std::cout << name << " | ms_float=" << msF << " ms_double=" << msD << " speedup=" << (msD / msF)
            << "x | max_abs=" << diff.maxAbs << " mean_abs=" << diff.meanAbs << " rmse=" << diff.rmse
            << " max_rel=" << diff.maxRel << '\n';
}

template <unsigned int Dim>
static void
RunAll(typename FImg<Dim>::Pointer input, int runs)
{
  input = PadMinSize<Dim>(input);
  const double sigma = 2.0;
  const auto   sz = input->GetLargestPossibleRegion().GetSize();

  std::cout << "\n=== precision_pipeline_bench dim=" << Dim << " size=";
  for (unsigned int d = 0; d < Dim; ++d)
  {
    if (d)
      std::cout << 'x';
    std::cout << sz[d];
  }
  std::cout << " runs=" << runs << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads()
            << " ===\n";

  typename FImg<Dim>::SizeType medRadius;
  medRadius.Fill(1);

  // 1) RecursiveGaussian
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::SmoothingRecursiveGaussianImageFilter<FImg<Dim>, FImg<Dim>>;
      auto s = F::New();
      s->SetInput(input);
      s->SetSigma(sigma);
      s->Update();
      return s->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::SmoothingRecursiveGaussianImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto s = F::New();
      s->SetInput(c->GetOutput());
      s->SetSigma(sigma);
      s->Update();
      return s->GetOutput();
    };
    runF();
    const double msF = TimeRuns<Dim>(runF, runs);
    const double msD = TimeRuns<Dim>(runD, runs);
    ReportCase<Dim>("RecursiveGaussian", msF, msD, DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 2) Median + RecursiveGaussian
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using Med = itk::MedianImageFilter<FImg<Dim>, FImg<Dim>>;
      using Sm = itk::SmoothingRecursiveGaussianImageFilter<FImg<Dim>, FImg<Dim>>;
      auto m = Med::New();
      m->SetRadius(medRadius);
      m->SetInput(input);
      auto s = Sm::New();
      s->SetInput(m->GetOutput());
      s->SetSigma(sigma);
      s->Update();
      return s->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using Med = itk::MedianImageFilter<DImg<Dim>, DImg<Dim>>;
      using Sm = itk::SmoothingRecursiveGaussianImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto m = Med::New();
      m->SetRadius(medRadius);
      m->SetInput(c->GetOutput());
      auto s = Sm::New();
      s->SetInput(m->GetOutput());
      s->SetSigma(sigma);
      s->Update();
      return s->GetOutput();
    };
    runF();
    const double msF = TimeRuns<Dim>(runF, runs);
    const double msD = TimeRuns<Dim>(runD, runs);
    ReportCase<Dim>("Median+RecursiveGaussian", msF, msD, DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 3) DiscreteGaussian
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::DiscreteGaussianImageFilter<FImg<Dim>, FImg<Dim>>;
      auto g = F::New();
      g->SetInput(input);
      g->SetVariance(sigma * sigma);
      g->SetMaximumError(0.01);
      g->Update();
      return g->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::DiscreteGaussianImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto g = F::New();
      g->SetInput(c->GetOutput());
      g->SetVariance(sigma * sigma);
      g->SetMaximumError(0.01);
      g->Update();
      return g->GetOutput();
    };
    runF();
    const double msF = TimeRuns<Dim>(runF, runs);
    const double msD = TimeRuns<Dim>(runD, runs);
    ReportCase<Dim>("DiscreteGaussian", msF, msD, DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 4) Otsu 分割
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using Otsu = itk::OtsuThresholdImageFilter<FImg<Dim>, FImg<Dim>>;
      auto o = Otsu::New();
      o->SetInput(input);
      o->SetInsideValue(1.0f);
      o->SetOutsideValue(0.0f);
      o->Update();
      return o->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using Otsu = itk::OtsuThresholdImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto o = Otsu::New();
      o->SetInput(c->GetOutput());
      o->SetInsideValue(1.0);
      o->SetOutsideValue(0.0);
      o->Update();
      return o->GetOutput();
    };
    runF();
    const double msF = TimeRuns<Dim>(runF, runs);
    const double msD = TimeRuns<Dim>(runD, runs);
    ReportCase<Dim>("OtsuThreshold", msF, msD, DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 5) ImageGradient
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::GradientMagnitudeImageFilter<FImg<Dim>, FImg<Dim>>;
      auto g = F::New();
      g->SetInput(input);
      g->Update();
      return g->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::GradientMagnitudeImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto g = F::New();
      g->SetInput(c->GetOutput());
      g->Update();
      return g->GetOutput();
    };
    runF();
    ReportCase<Dim>("GradientMagnitude", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 6) ImageIntensity
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::NormalizeImageFilter<FImg<Dim>, FImg<Dim>>;
      auto n = F::New();
      n->SetInput(input);
      n->Update();
      return n->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::NormalizeImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto n = F::New();
      n->SetInput(c->GetOutput());
      n->Update();
      return n->GetOutput();
    };
    runF();
    ReportCase<Dim>("Normalize", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 7) ImageGrid
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using Xf = itk::IdentityTransform<double, Dim>;
      using Interp = itk::LinearInterpolateImageFunction<FImg<Dim>, double>;
      using F = itk::ResampleImageFilter<FImg<Dim>, FImg<Dim>, double>;
      auto r = F::New();
      r->SetInput(input);
      r->SetTransform(Xf::New());
      r->SetInterpolator(Interp::New());
      r->SetSize(input->GetLargestPossibleRegion().GetSize());
      r->SetOutputSpacing(input->GetSpacing());
      r->SetOutputOrigin(input->GetOrigin());
      r->SetOutputDirection(input->GetDirection());
      r->Update();
      return r->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using Xf = itk::IdentityTransform<double, Dim>;
      using Interp = itk::LinearInterpolateImageFunction<DImg<Dim>, double>;
      using F = itk::ResampleImageFilter<DImg<Dim>, DImg<Dim>, double>;
      auto c = Cast::New();
      c->SetInput(input);
      auto r = F::New();
      r->SetInput(c->GetOutput());
      r->SetTransform(Xf::New());
      r->SetInterpolator(Interp::New());
      r->SetSize(input->GetLargestPossibleRegion().GetSize());
      r->SetOutputSpacing(input->GetSpacing());
      r->SetOutputOrigin(input->GetOrigin());
      r->SetOutputDirection(input->GetDirection());
      r->Update();
      return r->GetOutput();
    };
    runF();
    ReportCase<Dim>("ResampleIdentity", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 8) ImageFeature
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::SobelEdgeDetectionImageFilter<FImg<Dim>, FImg<Dim>>;
      auto s = F::New();
      s->SetInput(input);
      s->Update();
      return s->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::SobelEdgeDetectionImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto s = F::New();
      s->SetInput(c->GetOutput());
      s->Update();
      return s->GetOutput();
    };
    runF();
    ReportCase<Dim>("SobelEdge", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 9) MathematicalMorphology
  {
    using BallF = itk::BinaryBallStructuringElement<float, Dim>;
    using BallD = itk::BinaryBallStructuringElement<double, Dim>;
    BallF kf;
    kf.SetRadius(1);
    kf.CreateStructuringElement();
    BallD kd;
    kd.SetRadius(1);
    kd.CreateStructuringElement();
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::GrayscaleDilateImageFilter<FImg<Dim>, FImg<Dim>, BallF>;
      auto d = F::New();
      d->SetInput(input);
      d->SetKernel(kf);
      d->Update();
      return d->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::GrayscaleDilateImageFilter<DImg<Dim>, DImg<Dim>, BallD>;
      auto c = Cast::New();
      c->SetInput(input);
      auto d = F::New();
      d->SetInput(c->GetOutput());
      d->SetKernel(kd);
      d->Update();
      return d->GetOutput();
    };
    runF();
    ReportCase<Dim>("GrayscaleDilate", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 10) BinaryThreshold + DistanceMap
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using Th = itk::BinaryThresholdImageFilter<FImg<Dim>, FImg<Dim>>;
      auto t = Th::New();
      t->SetInput(input);
      t->SetLowerThreshold(50.0f);
      t->SetUpperThreshold(200.0f);
      t->SetInsideValue(1.0f);
      t->SetOutsideValue(0.0f);
      t->Update();
      return t->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using Th = itk::BinaryThresholdImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto t = Th::New();
      t->SetInput(c->GetOutput());
      t->SetLowerThreshold(50.0);
      t->SetUpperThreshold(200.0);
      t->SetInsideValue(1.0);
      t->SetOutsideValue(0.0);
      t->Update();
      return t->GetOutput();
    };
    runF();
    ReportCase<Dim>("BinaryThreshold", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using Th = itk::OtsuThresholdImageFilter<FImg<Dim>, FImg<Dim>>;
      using Dist = itk::SignedMaurerDistanceMapImageFilter<FImg<Dim>, FImg<Dim>>;
      auto t = Th::New();
      t->SetInput(input);
      t->SetInsideValue(1.0f);
      t->SetOutsideValue(0.0f);
      auto d = Dist::New();
      d->SetInput(t->GetOutput());
      d->SetInsideIsPositive(true);
      d->Update();
      return d->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using Th = itk::OtsuThresholdImageFilter<DImg<Dim>, DImg<Dim>>;
      using Dist = itk::SignedMaurerDistanceMapImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto t = Th::New();
      t->SetInput(c->GetOutput());
      t->SetInsideValue(1.0);
      t->SetOutsideValue(0.0);
      auto d = Dist::New();
      d->SetInput(t->GetOutput());
      d->SetInsideIsPositive(true);
      d->Update();
      return d->GetOutput();
    };
    runF();
    ReportCase<Dim>("SignedMaurerDistance", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }

  // 11) CurvatureFlow
  {
    auto runF = [&]() -> typename FImg<Dim>::Pointer {
      using F = itk::CurvatureFlowImageFilter<FImg<Dim>, FImg<Dim>>;
      auto f = F::New();
      f->SetInput(input);
      f->SetTimeStep(0.125);
      f->SetNumberOfIterations(8);
      f->Update();
      return f->GetOutput();
    };
    auto runD = [&]() -> typename DImg<Dim>::Pointer {
      using Cast = itk::CastImageFilter<FImg<Dim>, DImg<Dim>>;
      using F = itk::CurvatureFlowImageFilter<DImg<Dim>, DImg<Dim>>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = F::New();
      f->SetInput(c->GetOutput());
      f->SetTimeStep(0.125);
      f->SetNumberOfIterations(8);
      f->Update();
      return f->GetOutput();
    };
    runF();
    ReportCase<Dim>("CurvatureFlow", TimeRuns<Dim>(runF, runs), TimeRuns<Dim>(runD, runs),
                    DiffFD<Dim>(runD().GetPointer(), runF().GetPointer()));
  }
}

int
main(int argc, char ** argv)
{
  RegisterIO();
  const std::string path = argc >= 2 ? argv[1] : "";
  const int         runs = argc >= 3 ? std::stoi(argv[2]) : 5;

  if (!path.empty())
  {
    try
    {
      using Reader2 = itk::ImageFileReader<FImg<2>>;
      auto r2 = Reader2::New();
      r2->SetFileName(path);
      r2->UpdateOutputInformation();
      if (r2->GetImageIO()->GetNumberOfDimensions() <= 2)
      {
        r2->Update();
        RunAll<2>(r2->GetOutput(), runs);
        return 0;
      }
    }
    catch (...)
    {
    }
    try
    {
      using Reader3 = itk::ImageFileReader<FImg<3>>;
      auto r3 = Reader3::New();
      r3->SetFileName(path);
      r3->Update();
      RunAll<3>(r3->GetOutput(), runs);
      return 0;
    }
    catch (const std::exception & e)
    {
      std::cerr << "failed: " << e.what() << '\n';
      return 1;
    }
  }

  auto input = FImg<2>::New();
  {
    FImg<2>::SizeType   sz;
    FImg<2>::IndexType  idx;
    FImg<2>::RegionType reg;
    sz.Fill(1024);
    idx.Fill(0);
    reg.SetSize(sz);
    reg.SetIndex(idx);
    input->SetRegions(reg);
    input->Allocate();
    itk::ImageRegionIterator<FImg<2>> it(input, reg);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      const auto         id = it.GetIndex();
      const unsigned int x = static_cast<unsigned int>(id[0]);
      const unsigned int y = static_cast<unsigned int>(id[1]);
      it.Set(((x / 32) + (y / 32)) % 2 == 0 ? 0.0f : 1000.0f);
    }
  }
  RunAll<2>(input, runs);
  return 0;
}
