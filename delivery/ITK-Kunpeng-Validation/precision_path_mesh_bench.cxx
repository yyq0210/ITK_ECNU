// 造数据测 Path / QuadEdgeMesh：折线路径 + RegularSphere 球面网格。
// precision_path_mesh_bench --list
// precision_path_mesh_bench <image.png> [runs=3] [OperatorName]
#include "itkBorderQuadEdgeMeshFilter.h"
#include "itkCastImageFilter.h"
#include "itkChainCodePath.h"
#include "itkChainCodeToFourierSeriesPathFilter.h"
#include "itkCleanQuadEdgeMeshFilter.h"
#include "itkContourExtractor2DImageFilter.h"
#include "itkDelaunayConformingQuadEdgeMeshFilter.h"
#include "itkDiscreteGaussianCurvatureQuadEdgeMeshFilter.h"
#include "itkDiscreteMaximumCurvatureQuadEdgeMeshFilter.h"
#include "itkDiscreteMeanCurvatureQuadEdgeMeshFilter.h"
#include "itkDiscreteMinimumCurvatureQuadEdgeMeshFilter.h"
#include "itkExtractOrthogonalSwath2DImageFilter.h"
#include "itkFourierSeriesPath.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkPNGImageIOFactory.h"
#include "itkPathToChainCodePathFilter.h"
#include "itkPathToImageFilter.h"
#include "itkPolyLineParametricPath.h"
#include "itkQuadEdgeMesh.h"
#include "itkQuadEdgeMeshParamMatrixCoefficients.h"
#include "itkRegularSphereMeshSource.h"
#include "itkSmoothingQuadEdgeMeshFilter.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;
using Path2 = itk::PolyLineParametricPath<2>;
using MeshF = itk::QuadEdgeMesh<float, 3>;
using MeshD = itk::QuadEdgeMesh<double, 3>;

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

static Path2::Pointer
MakePolyline()
{
  auto p = Path2::New();
  p->Initialize();
  for (int i = 0; i <= 64; ++i)
  {
    Path2::ContinuousIndexType c;
    c[0] = 80.0 + 40.0 * std::cos(i * 0.1);
    c[1] = 90.0 + 30.0 * std::sin(i * 0.1);
    p->AddVertex(c);
  }
  return p;
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
  Path2::Pointer poly;
  MeshF::Pointer meshF;
  MeshD::Pointer meshD;

  if (!g_list)
  {
    using Reader = itk::ImageFileReader<FImg>;
    auto reader = Reader::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    input = reader->GetOutput();
    poly = MakePolyline();

    auto sf = itk::RegularSphereMeshSource<MeshF>::New();
    sf->SetResolution(4);
    sf->Update();
    meshF = sf->GetOutput();
    meshF->DisconnectPipeline();
    auto sd = itk::RegularSphereMeshSource<MeshD>::New();
    sd->SetResolution(4);
    sd->Update();
    meshD = sd->GetOutput();
    meshD->DisconnectPipeline();
    std::cerr << "path+mesh loaded img=" << input->GetLargestPossibleRegion().GetSize()[0] << " meshPts="
              << meshF->GetNumberOfPoints() << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads()
              << std::endl;
  }

  BenchTime(
    "Path",
    "ContourExtractor2DImageFilter",
    [&]() {
      auto f = itk::ContourExtractor2DImageFilter<FImg>::New();
      f->SetInput(input);
      f->SetContourValue(80.0);
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::ContourExtractor2DImageFilter<DImg>::New();
      f->SetInput(c->GetOutput());
      f->SetContourValue(80.0);
      f->Update();
    },
    runs);

  BenchTime(
    "Path",
    "PathToImageFilter",
    [&]() {
      auto f = itk::PathToImageFilter<Path2, FImg>::New();
      f->SetInput(poly);
      f->SetSize({ 256, 256 });
      f->Update();
    },
    [&]() {
      auto f = itk::PathToImageFilter<Path2, DImg>::New();
      f->SetInput(poly);
      f->SetSize({ 256, 256 });
      f->Update();
    },
    runs);

  BenchTime(
    "Path",
    "PathToChainCodePathFilter",
    [&]() {
      auto f = itk::PathToChainCodePathFilter<Path2, itk::ChainCodePath<2>>::New();
      f->SetInput(poly);
      f->Update();
    },
    [&]() {
      auto f = itk::PathToChainCodePathFilter<Path2, itk::ChainCodePath<2>>::New();
      f->SetInput(poly);
      f->Update();
    },
    runs);

  BenchTime(
    "Path",
    "ChainCodeToFourierSeriesPathFilter",
    [&]() {
      auto a = itk::PathToChainCodePathFilter<Path2, itk::ChainCodePath<2>>::New();
      a->SetInput(poly);
      auto f = itk::ChainCodeToFourierSeriesPathFilter<itk::ChainCodePath<2>, itk::FourierSeriesPath<2>>::New();
      f->SetInput(a->GetOutput());
      f->SetNumberOfHarmonics(8);
      f->Update();
    },
    [&]() {
      auto a = itk::PathToChainCodePathFilter<Path2, itk::ChainCodePath<2>>::New();
      a->SetInput(poly);
      auto f = itk::ChainCodeToFourierSeriesPathFilter<itk::ChainCodePath<2>, itk::FourierSeriesPath<2>>::New();
      f->SetInput(a->GetOutput());
      f->SetNumberOfHarmonics(8);
      f->Update();
    },
    runs);

  BenchTime(
    "Path",
    "ExtractOrthogonalSwath2DImageFilter",
    [&]() {
      auto f = itk::ExtractOrthogonalSwath2DImageFilter<FImg>::New();
      f->SetImageInput(input);
      f->SetPathInput(poly);
      f->SetSize({ 64, 32 });
      f->Update();
    },
    [&]() {
      using Cast = itk::CastImageFilter<FImg, DImg>;
      auto c = Cast::New();
      c->SetInput(input);
      auto f = itk::ExtractOrthogonalSwath2DImageFilter<DImg>::New();
      f->SetImageInput(c->GetOutput());
      f->SetPathInput(poly);
      f->SetSize({ 64, 32 });
      f->Update();
    },
    runs);

  itk::OnesMatrixCoefficients<MeshF> coeffF;
  itk::OnesMatrixCoefficients<MeshD> coeffD;

  BenchTime(
    "QuadEdgeMeshFiltering",
    "SmoothingQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::SmoothingQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->SetNumberOfIterations(4);
      f->SetRelaxationFactor(0.5);
      f->SetCoefficientsMethod(&coeffF);
      f->Update();
    },
    [&]() {
      auto f = itk::SmoothingQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->SetNumberOfIterations(4);
      f->SetRelaxationFactor(0.5);
      f->SetCoefficientsMethod(&coeffD);
      f->Update();
    },
    runs);

  BenchTime(
    "QuadEdgeMeshFiltering",
    "CleanQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::CleanQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->Update();
    },
    [&]() {
      auto f = itk::CleanQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->Update();
    },
    runs);

  BenchTime(
    "QuadEdgeMeshFiltering",
    "BorderQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::BorderQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->Update();
    },
    [&]() {
      auto f = itk::BorderQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->Update();
    },
    runs);

  BenchTime(
    "QuadEdgeMeshFiltering",
    "DelaunayConformingQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::DelaunayConformingQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->Update();
    },
    [&]() {
      auto f = itk::DelaunayConformingQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->Update();
    },
    runs);

  BenchTime(
    "QuadEdgeMeshFiltering",
    "DiscreteMeanCurvatureQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::DiscreteMeanCurvatureQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->Update();
    },
    [&]() {
      auto f = itk::DiscreteMeanCurvatureQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->Update();
    },
    runs);

  BenchTime(
    "QuadEdgeMeshFiltering",
    "DiscreteGaussianCurvatureQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::DiscreteGaussianCurvatureQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->Update();
    },
    [&]() {
      auto f = itk::DiscreteGaussianCurvatureQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->Update();
    },
    runs);

  BenchTime(
    "QuadEdgeMeshFiltering",
    "DiscreteMaximumCurvatureQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::DiscreteMaximumCurvatureQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->Update();
    },
    [&]() {
      auto f = itk::DiscreteMaximumCurvatureQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->Update();
    },
    runs);

  BenchTime(
    "QuadEdgeMeshFiltering",
    "DiscreteMinimumCurvatureQuadEdgeMeshFilter",
    [&]() {
      auto f = itk::DiscreteMinimumCurvatureQuadEdgeMeshFilter<MeshF, MeshF>::New();
      f->SetInput(meshF);
      f->Update();
    },
    [&]() {
      auto f = itk::DiscreteMinimumCurvatureQuadEdgeMeshFilter<MeshD, MeshD>::New();
      f->SetInput(meshD);
      f->Update();
    },
    runs);

  return 0;
}
