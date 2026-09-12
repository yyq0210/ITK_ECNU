// 造数据测水平集：脑图造速度场 + 圆心种子距离图当初值。2D，少迭代，只为可重复墙钟。
// precision_levelset_bench --list
// precision_levelset_bench <image.png> [runs=3] [OperatorName]
#include "itkAnisotropicFourthOrderLevelSetImageFilter.h"
#include "itkBinaryMaskToNarrowBandPointSetFilter.h"
#include "itkCannySegmentationLevelSetImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCollidingFrontsImageFilter.h"
#include "itkCurvesLevelSetImageFilter.h"
#include "itkExtensionVelocitiesImageFilter.h"
#include "itkGeodesicActiveContourLevelSetImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkIsotropicFourthOrderLevelSetImageFilter.h"
#include "itkLaplacianSegmentationLevelSetImageFilter.h"
#include "itkMesh.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkNarrowBandCurvesLevelSetImageFilter.h"
#include "itkNarrowBandThresholdSegmentationLevelSetImageFilter.h"
#include "itkPNGImageIOFactory.h"
#include "itkReinitializeLevelSetImageFilter.h"
#include "itkShapeDetectionLevelSetImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkThresholdSegmentationLevelSetImageFilter.h"
#include "itkUnsharpMaskLevelSetImageFilter.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;

static std::string g_only;
static bool        g_list = false;
static constexpr int kIters = 8;

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
MakeCircle(const TImage * ref, double radius)
{
  auto out = TImage::New();
  out->CopyInformation(ref);
  out->SetRegions(ref->GetLargestPossibleRegion());
  out->Allocate();
  out->FillBuffer(0);
  const auto sz = ref->GetLargestPossibleRegion().GetSize();
  const double cx = 0.5 * static_cast<double>(sz[0]);
  const double cy = 0.5 * static_cast<double>(sz[1]);
  itk::ImageRegionIterator<TImage> it(out, out->GetBufferedRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto   idx = it.GetIndex();
    const double dx = static_cast<double>(idx[0]) - cx;
    const double dy = static_cast<double>(idx[1]) - cy;
    if (dx * dx + dy * dy <= radius * radius)
    {
      it.Set(1);
    }
  }
  return out;
}

template <typename TIn, typename TOut>
static typename TOut::Pointer
Maurer(const TIn * bin)
{
  auto f = itk::SignedMaurerDistanceMapImageFilter<TIn, TOut>::New();
  f->SetInput(bin);
  f->SetInsideIsPositive(false);
  f->SetUseImageSpacing(false);
  f->Update();
  return Hold(f->GetOutput());
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

  FImg::Pointer input;
  FImg::Pointer initF;
  DImg::Pointer initD;
  FImg::Pointer speedF;
  DImg::Pointer speedD;
  FImg::Pointer seedF;
  DImg::Pointer seedD;
  FImg::Pointer collideF;
  DImg::Pointer collideD;
  FImg::IndexType seedIdx;
  FImg::IndexType seedIdx2;

  if (!g_list)
  {
    using Reader = itk::ImageFileReader<FImg>;
    auto reader = Reader::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    input = Hold(reader->GetOutput());
    const auto sz = input->GetLargestPossibleRegion().GetSize();
    seedIdx[0] = static_cast<long>(sz[0] / 2);
    seedIdx[1] = static_cast<long>(sz[1] / 2);
    seedIdx2 = seedIdx;
    seedIdx2[0] = static_cast<long>(sz[0] / 2 + std::min<long>(180, static_cast<long>(sz[0] / 4)));
    const double rad = 0.08 * static_cast<double>(std::min(sz[0], sz[1]));
    seedF = MakeCircle<FImg>(input, rad);
    using CastB = itk::CastImageFilter<FImg, DImg>;
    auto cb = CastB::New();
    cb->SetInput(seedF);
    cb->Update();
    seedD = Hold(cb->GetOutput());
    initF = Maurer<FImg, FImg>(seedF);
    initD = Maurer<DImg, DImg>(seedD);

    auto smooth = itk::SmoothingRecursiveGaussianImageFilter<FImg>::New();
    smooth->SetInput(input);
    smooth->SetSigma(1.0);
    auto grad = itk::GradientMagnitudeRecursiveGaussianImageFilter<FImg>::New();
    grad->SetInput(smooth->GetOutput());
    grad->SetSigma(1.0);
    auto sig = itk::SigmoidImageFilter<FImg, FImg>::New();
    sig->SetInput(grad->GetOutput());
    sig->SetAlpha(-0.5);
    sig->SetBeta(3.0);
    sig->SetOutputMinimum(0.0);
    sig->SetOutputMaximum(1.0);
    sig->Update();
    speedF = Hold(sig->GetOutput());
    using CastS = itk::CastImageFilter<FImg, DImg>;
    auto cs = CastS::New();
    cs->SetInput(speedF);
    cs->Update();
    speedD = Hold(cs->GetOutput());
    collideF = FImg::New();
    collideF->CopyInformation(input);
    collideF->SetRegions(input->GetLargestPossibleRegion());
    collideF->Allocate();
    collideF->FillBuffer(1.0f);
    collideD = DImg::New();
    collideD->CopyInformation(input);
    collideD->SetRegions(input->GetLargestPossibleRegion());
    collideD->Allocate();
    collideD->FillBuffer(1.0);
    std::cerr << "levelset " << sz[0] << 'x' << sz[1] << " rad=" << rad
              << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << std::endl;
  }

  BenchTime(
    "LevelSets",
    "ReinitializeLevelSetImageFilter",
    [&]() {
      auto f = itk::ReinitializeLevelSetImageFilter<FImg>::New();
      f->SetInput(initF);
      f->SetLevelSetValue(0.0);
      f->NarrowBandingOff();
      f->Update();
    },
    [&]() {
      auto f = itk::ReinitializeLevelSetImageFilter<DImg>::New();
      f->SetInput(initD);
      f->SetLevelSetValue(0.0);
      f->NarrowBandingOff();
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "GeodesicActiveContourLevelSetImageFilter",
    [&]() {
      auto f = itk::GeodesicActiveContourLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(speedF);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetAdvectionScaling(1.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      auto f = itk::GeodesicActiveContourLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(speedD);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetAdvectionScaling(1.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "ShapeDetectionLevelSetImageFilter",
    [&]() {
      auto f = itk::ShapeDetectionLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(speedF);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      auto f = itk::ShapeDetectionLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(speedD);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "ThresholdSegmentationLevelSetImageFilter",
    [&]() {
      auto f = itk::ThresholdSegmentationLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(input);
      f->SetUpperThreshold(200.0);
      f->SetLowerThreshold(40.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::ThresholdSegmentationLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(c->GetOutput());
      f->SetUpperThreshold(200.0);
      f->SetLowerThreshold(40.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "CurvesLevelSetImageFilter",
    [&]() {
      auto f = itk::CurvesLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(speedF);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetAdvectionScaling(1.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      auto f = itk::CurvesLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(speedD);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetAdvectionScaling(1.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "LaplacianSegmentationLevelSetImageFilter",
    [&]() {
      auto f = itk::LaplacianSegmentationLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(input);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::LaplacianSegmentationLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(c->GetOutput());
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "CannySegmentationLevelSetImageFilter",
    [&]() {
      auto f = itk::CannySegmentationLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(input);
      f->SetThreshold(7.0);
      f->SetVariance(0.1);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::CannySegmentationLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(c->GetOutput());
      f->SetThreshold(7.0);
      f->SetVariance(0.1);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "NarrowBandThresholdSegmentationLevelSetImageFilter",
    [&]() {
      auto f = itk::NarrowBandThresholdSegmentationLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(input);
      f->SetUpperThreshold(200.0);
      f->SetLowerThreshold(40.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::NarrowBandThresholdSegmentationLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(c->GetOutput());
      f->SetUpperThreshold(200.0);
      f->SetLowerThreshold(40.0);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "NarrowBandCurvesLevelSetImageFilter",
    [&]() {
      auto f = itk::NarrowBandCurvesLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetFeatureImage(speedF);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      auto f = itk::NarrowBandCurvesLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetFeatureImage(speedD);
      f->SetPropagationScaling(1.0);
      f->SetCurvatureScaling(0.5);
      f->SetMaximumRMSError(0.02);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "CollidingFrontsImageFilter",
    [&]() {
      using F = itk::CollidingFrontsImageFilter<FImg, FImg>;
      using Nodes = F::NodeContainer;
      using Node = F::NodeType;
      auto s1 = Nodes::New();
      auto s2 = Nodes::New();
      Node n;
      n.SetValue(0.0);
      n.SetIndex(seedIdx);
      s1->Initialize();
      s1->InsertElement(0, n);
      n.SetIndex(seedIdx2);
      s2->Initialize();
      s2->InsertElement(0, n);
      auto f = F::New();
      f->SetInput(collideF);
      f->SetSeedPoints1(s1);
      f->SetSeedPoints2(s2);
      f->ApplyConnectivityOff();
      f->Update();
    },
    [&]() {
      using F = itk::CollidingFrontsImageFilter<DImg, DImg>;
      using Nodes = F::NodeContainer;
      using Node = F::NodeType;
      auto s1 = Nodes::New();
      auto s2 = Nodes::New();
      Node n;
      n.SetValue(0.0);
      n.SetIndex(seedIdx);
      s1->Initialize();
      s1->InsertElement(0, n);
      n.SetIndex(seedIdx2);
      s2->Initialize();
      s2->InsertElement(0, n);
      auto f = F::New();
      f->SetInput(collideD);
      f->SetSeedPoints1(s1);
      f->SetSeedPoints2(s2);
      f->ApplyConnectivityOff();
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "UnsharpMaskLevelSetImageFilter",
    [&]() {
      auto f = itk::UnsharpMaskLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetNumberOfIterations(kIters);
      f->Update();
    },
    [&]() {
      auto f = itk::UnsharpMaskLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetNumberOfIterations(kIters);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "AnisotropicFourthOrderLevelSetImageFilter",
    [&]() {
      auto f = itk::AnisotropicFourthOrderLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      auto f = itk::AnisotropicFourthOrderLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "IsotropicFourthOrderLevelSetImageFilter",
    [&]() {
      auto f = itk::IsotropicFourthOrderLevelSetImageFilter<FImg, FImg>::New();
      f->SetInput(initF);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    [&]() {
      auto f = itk::IsotropicFourthOrderLevelSetImageFilter<DImg, DImg>::New();
      f->SetInput(initD);
      f->SetNumberOfIterations(kIters);
      f->SetIsoSurfaceValue(0.0);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "BinaryMaskToNarrowBandPointSetFilter",
    [&]() {
      auto f = itk::BinaryMaskToNarrowBandPointSetFilter<FImg, itk::Mesh<float, 2>>::New();
      f->SetInput(seedF);
      f->SetBandWidth(2.5);
      f->Update();
    },
    [&]() {
      auto f = itk::BinaryMaskToNarrowBandPointSetFilter<DImg, itk::Mesh<double, 2>>::New();
      f->SetInput(seedD);
      f->SetBandWidth(2.5);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "ExtensionVelocitiesImageFilter",
    [&]() {
      auto f = itk::ExtensionVelocitiesImageFilter<FImg, float, 1>::New();
      f->SetInput(initF);
      f->SetInputVelocityImage(speedF);
      f->SetLevelSetValue(0.0);
      f->NarrowBandingOff();
      f->Update();
    },
    [&]() {
      auto f = itk::ExtensionVelocitiesImageFilter<DImg, double, 1>::New();
      f->SetInput(initD);
      f->SetInputVelocityImage(speedD);
      f->SetLevelSetValue(0.0);
      f->NarrowBandingOff();
      f->Update();
    },
    runs);

  return 0;
}
