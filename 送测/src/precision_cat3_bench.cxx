// 造第 3 类合法输入：开边界 QuadEdge、Simplex、路径+merit、形状先验、
// SpatialObject、标签点集、LevelSetsv4 domain map。
// FEM 模块本机构建 OFF，不编。
// precision_cat3_bench --list
// precision_cat3_bench <image.png> [runs=3] [OperatorName]
#include "VNLSparseLUSolverTraits.h"
#include "itkAmoebaOptimizer.h"
#include "itkBorderQuadEdgeMeshFilter.h"
#include "itkCastImageFilter.h"
#include "itkChainCodePath.h"
#include "itkChainCodeToFourierSeriesPathFilter.h"
#include "itkDefaultDynamicMeshTraits.h"
#include "itkDeformableSimplexMesh3DBalloonForceFilter.h"
#include "itkDeformableSimplexMesh3DFilter.h"
#include "itkDeformableSimplexMesh3DGradientConstraintForceFilter.h"
#include "itkDerivativeImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkEllipseSpatialObject.h"
#include "itkEuler2DTransform.h"
#include "itkExtractOrthogonalSwath2DImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkFourierSeriesPath.h"
#include "itkGeodesicActiveContourShapePriorLevelSetImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkGradientRecursiveGaussianImageFilter.h"
#include "itkGroupSpatialObject.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageToSpatialObjectMetric.h"
#include "itkImageToSpatialObjectRegistrationMethod.h"
#include "itkLabeledPointSetToPointSetMetricv4.h"
#include "itkLaplacianDeformationQuadEdgeMeshFilterWithHardConstraints.h"
#include "itkLaplacianDeformationQuadEdgeMeshFilterWithSoftConstraints.h"
#include "itkLevelSetDomainMapImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMesh.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkNormalQuadEdgeMeshFilter.h"
#include "itkNormalVariateGenerator.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"
#include "itkOrthogonalSwath2DPathFilter.h"
#include "itkPNGImageIOFactory.h"
#include "itkParameterizationQuadEdgeMeshFilter.h"
#include "itkPathToChainCodePathFilter.h"
#include "itkPointSet.h"
#include "itkPolyLineParametricPath.h"
#include "itkQuadEdgeMesh.h"
#include "itkQuadEdgeMeshDecimationCriteria.h"
#include "itkQuadEdgeMeshExtendedTraits.h"
#include "itkQuadEdgeMeshParamMatrixCoefficients.h"
#include "itkQuadricDecimationQuadEdgeMeshFilter.h"
#include "itkRegularSphereMeshSource.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkShapePriorMAPCostFunction.h"
#include "itkSigmoidImageFilter.h"
#include "itkSimplexMesh.h"
#include "itkSobelEdgeDetectionImageFilter.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkSphereSignedDistanceFunction.h"
#include "itkSquaredEdgeLengthDecimationQuadEdgeMeshFilter.h"
#include "itkTranslationTransform.h"
#include "itkTriangleMeshToSimplexMeshFilter.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;
using MeshF = itk::QuadEdgeMesh<float, 3>;
using MeshD = itk::QuadEdgeMesh<double, 3>;
using Vec3F = itk::Vector<float, 3>;
using Vec3D = itk::Vector<double, 3>;
using NTraitsF = itk::QuadEdgeMeshExtendedTraits<Vec3F, 3, 2, float, float, Vec3F, bool, bool>;
using NTraitsD = itk::QuadEdgeMeshExtendedTraits<Vec3D, 3, 2, double, double, Vec3D, bool, bool>;
using NMeshF = itk::QuadEdgeMesh<Vec3F, 3, NTraitsF>;
using NMeshD = itk::QuadEdgeMesh<Vec3D, 3, NTraitsD>;
using TriTraits = itk::DefaultDynamicMeshTraits<double, 3, 3, double, double>;
using SxTraits = itk::DefaultDynamicMeshTraits<double, 3, 3, double, double>;
using TriMesh = itk::Mesh<double, 3, TriTraits>;
using SxMesh = itk::SimplexMesh<double, 3, SxTraits>;
using Img3F = itk::Image<float, 3>;
using Path2 = itk::PolyLineParametricPath<2>;
using Chain2 = itk::ChainCodePath<2>;
using Fourier2 = itk::FourierSeriesPath<2>;

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

template <typename TMesh>
static typename TMesh::Pointer
MakeDiskQE(unsigned n = 14)
{
  auto mesh = TMesh::New();
  using PointType = typename TMesh::PointType;
  using Id = typename TMesh::PointIdentifier;
  using Coord = typename PointType::ValueType;
  Id id = 0;
  for (unsigned j = 0; j < n; ++j)
  {
    for (unsigned i = 0; i < n; ++i)
    {
      PointType p;
      p[0] = static_cast<Coord>(i);
      p[1] = static_cast<Coord>(j);
      p[2] = static_cast<Coord>(0.15 * std::sin(0.4 * i) * std::cos(0.4 * j));
      mesh->SetPoint(id++, p);
    }
  }
  for (unsigned j = 0; j + 1 < n; ++j)
  {
    for (unsigned i = 0; i + 1 < n; ++i)
    {
      const Id a = static_cast<Id>(j * n + i);
      const Id b = a + 1;
      const Id c = a + n;
      const Id d = c + 1;
      mesh->AddFaceTriangle(a, b, d);
      mesh->AddFaceTriangle(a, d, c);
    }
  }
  return mesh;
}

template <typename TMesh>
static typename TMesh::Pointer
MakeSphereQE(unsigned res = 3)
{
  auto src = itk::RegularSphereMeshSource<TMesh>::New();
  src->SetResolution(res);
  src->Update();
  return Hold(src->GetOutput());
}

template <typename TPixel>
static void
RunBorder(int)
{
  using Mesh = itk::QuadEdgeMesh<TPixel, 3>;
  auto mesh = MakeDiskQE<Mesh>(14);
  auto f = itk::BorderQuadEdgeMeshFilter<Mesh, Mesh>::New();
  f->SetInput(mesh);
  f->SetTransformType(itk::BorderQuadEdgeMeshFilterEnums::BorderTransform::SQUARE_BORDER_TRANSFORM);
  f->SetBorderPick(itk::BorderQuadEdgeMeshFilterEnums::BorderPick::LONGEST);
  f->Update();
}

template <typename TPixel>
static void
RunParameterization(int)
{
  using Mesh = itk::QuadEdgeMesh<TPixel, 3>;
  using Solver = VNLSparseLUSolverTraits<TPixel>;
  auto mesh = MakeDiskQE<Mesh>(12);
  auto border = itk::BorderQuadEdgeMeshFilter<Mesh, Mesh>::New();
  border->SetInput(mesh);
  border->SetTransformType(itk::BorderQuadEdgeMeshFilterEnums::BorderTransform::SQUARE_BORDER_TRANSFORM);
  border->SetBorderPick(itk::BorderQuadEdgeMeshFilterEnums::BorderPick::LONGEST);
  itk::OnesMatrixCoefficients<Mesh> coeff;
  auto                              f = itk::ParameterizationQuadEdgeMeshFilter<Mesh, Mesh, Solver>::New();
  f->SetInput(mesh);
  f->SetBorderTransform(border);
  f->SetCoefficientsMethod(&coeff);
  f->Update();
}

template <typename TPixel>
static void
RunNormal(int)
{
  using Mesh = itk::QuadEdgeMesh<TPixel, 3>;
  using Vec = itk::Vector<TPixel, 3>;
  using Traits = itk::QuadEdgeMeshExtendedTraits<Vec, 3, 2, TPixel, TPixel, Vec, bool, bool>;
  using Out = itk::QuadEdgeMesh<Vec, 3, Traits>;
  auto mesh = MakeSphereQE<Mesh>(3);
  auto f = itk::NormalQuadEdgeMeshFilter<Mesh, Out>::New();
  f->SetInput(mesh);
  f->SetWeight(itk::NormalQuadEdgeMeshFilter<Mesh, Out>::WeightEnum::GOURAUD);
  f->Update();
}

template <typename TPixel>
static void
RunQuadric(int)
{
  using Mesh = itk::QuadEdgeMesh<TPixel, 3>;
  using Crit = itk::NumberOfFacesCriterion<Mesh>;
  auto mesh = MakeSphereQE<Mesh>(3);
  auto crit = Crit::New();
  crit->SetTopologicalChange(true);
  const auto n = mesh->GetNumberOfCells();
  crit->SetNumberOfElements(n > 16 ? n / 2 : 8);
  auto f = itk::QuadricDecimationQuadEdgeMeshFilter<Mesh, Mesh, Crit>::New();
  f->SetInput(mesh);
  f->SetCriterion(crit);
  f->Update();
}

template <typename TPixel>
static void
RunSquaredDecim(int)
{
  using Mesh = itk::QuadEdgeMesh<TPixel, 3>;
  using Crit = itk::NumberOfFacesCriterion<Mesh>;
  auto mesh = MakeSphereQE<Mesh>(3);
  auto crit = Crit::New();
  crit->SetTopologicalChange(true);
  const auto n = mesh->GetNumberOfCells();
  crit->SetNumberOfElements(n > 16 ? n / 2 : 8);
  auto f = itk::SquaredEdgeLengthDecimationQuadEdgeMeshFilter<Mesh, Mesh, Crit>::New();
  f->SetInput(mesh);
  f->SetCriterion(crit);
  f->Update();
}

template <typename TPixel>
static void
RunLapHard(int)
{
  using Mesh = itk::QuadEdgeMesh<TPixel, 3>;
  using Solver = VNLSparseLUSolverTraits<TPixel>;
  using Filt = itk::LaplacianDeformationQuadEdgeMeshFilterWithHardConstraints<Mesh, Mesh, Solver>;
  auto                            mesh = MakeSphereQE<Mesh>(3);
  itk::ConformalMatrixCoefficients<Mesh> coeff;
  auto                                   f = Filt::New();
  f->SetInput(mesh);
  f->SetOrder(1);
  f->SetAreaComputationType(Filt::AreaEnum::NONE);
  f->SetCoefficientsMethod(&coeff);
  typename Mesh::VectorType keep{};
  typename Mesh::VectorType down{};
  down[2] = static_cast<TPixel>(-0.2);
  f->SetDisplacement(0, keep);
  f->SetDisplacement(mesh->GetNumberOfPoints() / 2, down);
  f->Update();
}

template <typename TPixel>
static void
RunLapSoft(int)
{
  using Mesh = itk::QuadEdgeMesh<TPixel, 3>;
  using Solver = VNLSparseLUSolverTraits<TPixel>;
  using Filt = itk::LaplacianDeformationQuadEdgeMeshFilterWithSoftConstraints<Mesh, Mesh, Solver>;
  auto                            mesh = MakeSphereQE<Mesh>(3);
  itk::ConformalMatrixCoefficients<Mesh> coeff;
  auto                                   f = Filt::New();
  f->SetInput(mesh);
  f->SetOrder(1);
  f->SetLambda(1.0);
  f->SetAreaComputationType(Filt::AreaEnum::NONE);
  f->SetCoefficientsMethod(&coeff);
  typename Mesh::VectorType keep{};
  typename Mesh::VectorType down{};
  down[2] = static_cast<TPixel>(-0.2);
  const auto mid = mesh->GetNumberOfPoints() / 2;
  f->SetDisplacement(0, keep);
  f->SetDisplacement(mid, down);
  f->SetLocalLambda(mid, 0.1);
  f->Update();
}

static Path2::Pointer
MakeSquarePath()
{
  auto p = Path2::New();
  Path2::ContinuousIndexType v;
  v[0] = 24;
  v[1] = 24;
  p->AddVertex(v);
  v[1] = 104;
  p->AddVertex(v);
  v[0] = 104;
  p->AddVertex(v);
  v[1] = 24;
  p->AddVertex(v);
  v[0] = 24;
  p->AddVertex(v);
  return p;
}

template <typename TPixel>
static void
RunOrthogonalSwath(int)
{
  using Img = itk::Image<TPixel, 2>;
  using U8 = itk::Image<unsigned char, 2>;
  auto inputPath = MakeSquarePath();
  auto toChain = itk::PathToChainCodePathFilter<Path2, Chain2>::New();
  toChain->SetInput(inputPath);
  auto toFourier = itk::ChainCodeToFourierSeriesPathFilter<Chain2, Fourier2>::New();
  toFourier->SetInput(toChain->GetOutput());
  toFourier->SetNumberOfHarmonics(8);

  auto u8 = U8::New();
  typename U8::IndexType i0;
  i0.Fill(0);
  typename U8::SizeType sz;
  sz.Fill(128);
  u8->SetRegions({ i0, sz });
  u8->Allocate();
  itk::ImageRegionIteratorWithIndex<U8> it(u8, u8->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto ix = it.GetIndex();
    const bool inner = ix[0] >= 32 && ix[0] < 96 && ix[1] >= 32 && ix[1] < 96;
    it.Set(inner ? 0 : 255);
  }
  using Cast = itk::RescaleIntensityImageFilter<U8, Img>;
  auto cast = Cast::New();
  cast->SetInput(u8);
  cast->SetOutputMinimum(0);
  cast->SetOutputMaximum(1);
  auto smooth = itk::DiscreteGaussianImageFilter<Img, Img>::New();
  smooth->SetInput(cast->GetOutput());
  smooth->SetVariance(1.0);
  smooth->SetMaximumError(0.9);
  smooth->UseImageSpacingOff();
  auto swath = itk::ExtractOrthogonalSwath2DImageFilter<Img>::New();
  swath->SetImageInput(smooth->GetOutput());
  swath->SetPathInput(toFourier->GetOutput());
  typename Img::SizeType ssz;
  ssz[0] = 256;
  ssz[1] = 33;
  swath->SetSize(ssz);
  auto merit = itk::DerivativeImageFilter<Img, Img>::New();
  merit->SetInput(swath->GetOutput());
  merit->SetOrder(1);
  merit->SetDirection(1);
  merit->Update();
  auto meritImg = Hold(merit->GetOutput());
  toFourier->Update();
  auto fourier = toFourier->GetOutput();
  fourier->DisconnectPipeline();

  auto f = itk::OrthogonalSwath2DPathFilter<Fourier2, Img>::New();
  f->SetPathInput(fourier);
  f->SetImageInput(meritImg);
  f->Update();
}

static void
RunDomainMap(int)
{
  using ListPixel = std::list<int>;
  using In = itk::Image<ListPixel, 2>;
  using Out = itk::Image<unsigned short, 2>;
  auto img = In::New();
  In::IndexType idx;
  idx.Fill(0);
  In::SizeType sz;
  sz.Fill(32);
  img->SetRegions({ idx, sz });
  img->Allocate();
  img->FillBuffer(ListPixel{});
  for (unsigned i = 0; i < 32; ++i)
  {
    ListPixel ll;
    ll.push_back(static_cast<int>(i % 4));
    ll.push_back(static_cast<int>((i % 4) + 1));
    In::IndexType p;
    p[0] = static_cast<long>(i);
    p[1] = static_cast<long>(i);
    img->SetPixel(p, ll);
  }
  auto f = itk::LevelSetDomainMapImageFilter<In, Out>::New();
  f->SetInput(img);
  f->Update();
}

static void
RunLabeledPointSet(int)
{
  using PS = itk::PointSet<int, 3>;
  auto fix = PS::New();
  auto mov = PS::New();
  for (int i = 0; i < 40; ++i)
  {
    PS::PointType p;
    p[0] = 0.4 * i;
    p[1] = 0.15 * (i % 7);
    p[2] = 0.05 * i;
    const int lab = (i < 20) ? 1 : 2;
    fix->SetPoint(i, p);
    fix->SetPointData(i, lab);
    p[0] += 0.12;
    p[1] += 0.04;
    mov->SetPoint(i, p);
    mov->SetPointData(i, lab);
  }
  auto metric = itk::LabeledPointSetToPointSetMetricv4<PS>::New();
  auto trans = itk::TranslationTransform<double, 3>::New();
  trans->SetIdentity();
  metric->SetFixedPointSet(fix);
  metric->SetMovingPointSet(mov);
  metric->SetMovingTransform(trans);
  metric->Initialize();
  (void)metric->GetValue();
}

template <typename TPixel>
static void
RunShapePrior(int)
{
  using Img = itk::Image<TPixel, 2>;
  constexpr unsigned n = 48;
  auto               raw = Img::New();
  typename Img::IndexType i0;
  i0.Fill(0);
  typename Img::SizeType sz;
  sz.Fill(n);
  raw->SetRegions({ i0, sz });
  raw->Allocate();
  const double cx = 24.0;
  const double cy = 24.0;
  const double rad = 12.0;
  itk::ImageRegionIteratorWithIndex<Img> it(raw, raw->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const double dx = it.GetIndex()[0] - cx;
    const double dy = it.GetIndex()[1] - cy;
    it.Set(static_cast<TPixel>(std::sqrt(dx * dx + dy * dy) < rad ? 190.0 : 0.0));
  }
  auto gm = itk::GradientMagnitudeRecursiveGaussianImageFilter<Img, Img>::New();
  gm->SetInput(raw);
  gm->SetSigma(1.0);
  auto sig = itk::SigmoidImageFilter<Img, Img>::New();
  sig->SetInput(gm->GetOutput());
  sig->SetOutputMinimum(0.0);
  sig->SetOutputMaximum(1.0);
  sig->SetAlpha(-0.4);
  sig->SetBeta(2.5);
  using FM = itk::FastMarchingImageFilter<Img>;
  auto fm = FM::New();
  auto seeds = FM::NodeContainer::New();
  typename FM::NodeType node;
  node.SetValue(-6.0);
  typename Img::IndexType seed;
  seed.Fill(22);
  node.SetIndex(seed);
  seeds->Initialize();
  seeds->InsertElement(0, node);
  fm->SetTrialPoints(seeds);
  fm->SetSpeedConstant(1.0);
  fm->SetOutputSize(sz);

  using Filt = itk::GeodesicActiveContourShapePriorLevelSetImageFilter<Img, Img, TPixel>;
  using Shape = itk::SphereSignedDistanceFunction<double, 2>;
  using Cost = itk::ShapePriorMAPCostFunction<Img, TPixel>;
  auto filter = Filt::New();
  auto shape = Shape::New();
  auto cost = Cost::New();
  auto opt = itk::AmoebaOptimizer::New();
  shape->Initialize();
  typename Cost::ArrayType mean(shape->GetNumberOfShapeParameters());
  typename Cost::ArrayType stddev(shape->GetNumberOfShapeParameters());
  mean[0] = 10.0;
  stddev[0] = 2.0;
  cost->SetShapeParameterMeans(mean);
  cost->SetShapeParameterStandardDeviations(stddev);
  typename Cost::WeightsType weights;
  weights.Fill(1.0);
  weights[1] = 10.0;
  cost->SetWeights(weights);
  opt->SetFunctionConvergenceTolerance(0.1);
  opt->SetParametersConvergenceTolerance(0.5);
  opt->SetMaximumNumberOfIterations(6);
  typename Filt::ParametersType parameters(shape->GetNumberOfParameters());
  parameters[0] = mean[0];
  parameters[1] = 24;
  parameters[2] = 24;
  filter->SetPropagationScaling(0.5);
  filter->SetAdvectionScaling(1.0);
  filter->SetCurvatureScaling(1.0);
  filter->SetShapePriorScaling(0.1);
  filter->SetInput(fm->GetOutput());
  filter->SetFeatureImage(sig->GetOutput());
  filter->SetShapeFunction(shape);
  filter->SetCostFunction(cost);
  filter->SetOptimizer(opt);
  filter->SetInitialParameters(parameters);
  filter->SetNumberOfLayers(3);
  filter->SetMaximumRMSError(0.05);
  filter->SetNumberOfIterations(6);
  filter->Update();
}

template <typename TPixel>
static void
RunSimplexBase(int)
{
  auto src = itk::RegularSphereMeshSource<TriMesh>::New();
  typename TriMesh::PointType center;
  center.Fill(10);
  typename itk::RegularSphereMeshSource<TriMesh>::VectorType scale;
  scale.Fill(3);
  src->SetCenter(center);
  src->SetResolution(2);
  src->SetScale(scale);
  auto toSx = itk::TriangleMeshToSimplexMeshFilter<TriMesh, SxMesh>::New();
  toSx->SetInput(src->GetOutput());
  toSx->Update();
  auto sx = Hold(toSx->GetOutput());

  auto box = Img3F::New();
  Img3F::SizeType bsz;
  bsz.Fill(20);
  box->SetRegions(bsz);
  box->Allocate();
  box->FillBuffer(0);
  itk::ImageRegionIteratorWithIndex<Img3F> it(box, box->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto x = it.GetIndex();
    const bool wall = ((x[0] == 5 || x[0] == 15) && x[1] >= 5 && x[1] <= 15 && x[2] >= 5 && x[2] <= 15) ||
                      ((x[1] == 5 || x[1] == 15) && x[0] >= 5 && x[0] <= 15 && x[2] >= 5 && x[2] <= 15) ||
                      ((x[2] == 5 || x[2] == 15) && x[1] >= 5 && x[1] <= 15 && x[0] >= 5 && x[0] <= 15);
    if (wall)
    {
      it.Set(1);
    }
  }
  auto gad = itk::GradientAnisotropicDiffusionImageFilter<Img3F, Img3F>::New();
  gad->SetInput(box);
  gad->SetNumberOfIterations(2);
  gad->SetTimeStep(0.0625);
  gad->SetConductanceParameter(3);
  auto gmag = itk::GradientMagnitudeRecursiveGaussianImageFilter<Img3F, Img3F>::New();
  gmag->SetInput(gad->GetOutput());
  gmag->SetSigma(1.0);
  auto sig = itk::SigmoidImageFilter<Img3F, Img3F>::New();
  sig->SetInput(gmag->GetOutput());
  sig->SetOutputMinimum(0);
  sig->SetOutputMaximum(1);
  sig->SetAlpha(10);
  sig->SetBeta(100);
  using Def = itk::DeformableSimplexMesh3DFilter<SxMesh, SxMesh>;
  auto grad = itk::GradientRecursiveGaussianImageFilter<Img3F, typename Def::GradientImageType>::New();
  grad->SetInput(sig->GetOutput());
  grad->SetSigma(1.0);
  grad->Update();
  auto f = Def::New();
  f->SetInput(sx);
  f->SetGradient(grad->GetOutput());
  f->SetAlpha(0.1);
  f->SetBeta(-0.1);
  f->SetGamma(0.05);
  f->SetDamping(0.65);
  f->SetIterations(3);
  f->SetRigidity(1);
  f->Update();
}

template <typename TPixel>
static void
RunSimplexBalloon(int)
{
  auto src = itk::RegularSphereMeshSource<TriMesh>::New();
  typename TriMesh::PointType center;
  center.Fill(10);
  typename itk::RegularSphereMeshSource<TriMesh>::VectorType scale;
  scale.Fill(3);
  src->SetCenter(center);
  src->SetResolution(2);
  src->SetScale(scale);
  auto toSx = itk::TriangleMeshToSimplexMeshFilter<TriMesh, SxMesh>::New();
  toSx->SetInput(src->GetOutput());
  auto box = Img3F::New();
  Img3F::SizeType bsz;
  bsz.Fill(20);
  box->SetRegions(bsz);
  box->Allocate();
  box->FillBuffer(0);
  itk::ImageRegionIteratorWithIndex<Img3F> it(box, box->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto x = it.GetIndex();
    const bool wall = ((x[0] == 5 || x[0] == 15) && x[1] >= 5 && x[1] <= 15 && x[2] >= 5 && x[2] <= 15) ||
                      ((x[1] == 5 || x[1] == 15) && x[0] >= 5 && x[0] <= 15 && x[2] >= 5 && x[2] <= 15) ||
                      ((x[2] == 5 || x[2] == 15) && x[1] >= 5 && x[1] <= 15 && x[0] >= 5 && x[0] <= 15);
    if (wall)
    {
      it.Set(1);
    }
  }
  auto edge = itk::SobelEdgeDetectionImageFilter<Img3F, Img3F>::New();
  edge->SetInput(box);
  using Def = itk::DeformableSimplexMesh3DBalloonForceFilter<SxMesh, SxMesh>;
  auto grad = itk::GradientRecursiveGaussianImageFilter<Img3F, typename Def::GradientImageType>::New();
  grad->SetInput(edge->GetOutput());
  grad->SetSigma(1.0);
  auto f = Def::New();
  f->SetInput(toSx->GetOutput());
  f->SetGradient(grad->GetOutput());
  f->SetAlpha(0.2);
  f->SetBeta(0.1);
  f->SetKappa(0.2);
  f->SetIterations(4);
  f->SetRigidity(0);
  f->Update();
}

template <typename TPixel>
static void
RunSimplexGrad(int)
{
  auto src = itk::RegularSphereMeshSource<TriMesh>::New();
  typename TriMesh::PointType center;
  center.Fill(10);
  typename itk::RegularSphereMeshSource<TriMesh>::VectorType scale;
  scale.Fill(5);
  src->SetCenter(center);
  src->SetResolution(2);
  src->SetScale(scale);
  auto toSx = itk::TriangleMeshToSimplexMeshFilter<TriMesh, SxMesh>::New();
  toSx->SetInput(src->GetOutput());
  auto box = Img3F::New();
  Img3F::SizeType bsz;
  bsz.Fill(20);
  box->SetRegions(bsz);
  box->Allocate();
  box->FillBuffer(0);
  itk::ImageRegionIteratorWithIndex<Img3F> it(box, box->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto x = it.GetIndex();
    const bool wall = ((x[0] == 5 || x[0] == 15) && x[1] >= 5 && x[1] <= 15 && x[2] >= 5 && x[2] <= 15) ||
                      ((x[1] == 5 || x[1] == 15) && x[0] >= 5 && x[0] <= 15 && x[2] >= 5 && x[2] <= 15) ||
                      ((x[2] == 5 || x[2] == 15) && x[1] >= 5 && x[1] <= 15 && x[0] >= 5 && x[0] <= 15);
    if (wall)
    {
      it.Set(1);
    }
  }
  auto edge = itk::SobelEdgeDetectionImageFilter<Img3F, Img3F>::New();
  edge->SetInput(box);
  using Def = itk::DeformableSimplexMesh3DGradientConstraintForceFilter<SxMesh, SxMesh>;
  auto grad = itk::GradientRecursiveGaussianImageFilter<Img3F, typename Def::GradientImageType>::New();
  grad->SetInput(edge->GetOutput());
  grad->SetSigma(1.0);
  auto f = Def::New();
  f->SetInput(toSx->GetOutput());
  f->SetImage(box);
  f->SetGradient(grad->GetOutput());
  f->SetAlpha(0.2);
  f->SetBeta(0.1);
  f->SetRange(1);
  f->SetIterations(4);
  f->SetRigidity(0);
  f->Update();
}

template <typename TFixedImage, typename TMovingSpatialObject>
class SimpleSOMetric : public itk::ImageToSpatialObjectMetric<TFixedImage, TMovingSpatialObject>
{
public:
  using Self = SimpleSOMetric;
  using Superclass = itk::ImageToSpatialObjectMetric<TFixedImage, TMovingSpatialObject>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;
  itkNewMacro(Self);
  itkOverrideGetNameOfClassMacro(SimpleSOMetric);
  using ParametersType = typename Superclass::ParametersType;
  using DerivativeType = typename Superclass::DerivativeType;
  using MeasureType = typename Superclass::MeasureType;
  using PointType = itk::Point<double, 2>;

  MeasureType
  GetValue(const ParametersType & parameters) const override
  {
    this->m_Transform->SetParameters(parameters);
    double value = 0.0;
    itk::ImageRegionConstIteratorWithIndex<TFixedImage> it(this->m_FixedImage,
                                                           this->m_FixedImage->GetLargestPossibleRegion());
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      PointType soPt;
      this->m_FixedImage->TransformIndexToPhysicalPoint(it.GetIndex(), soPt);
      if (!this->m_MovingSpatialObject->IsInsideInWorldSpace(soPt))
      {
        continue;
      }
      const auto imgPt = this->m_Transform->TransformPoint(soPt);
      typename TFixedImage::IndexType index;
      if (!this->m_FixedImage->TransformPhysicalPointToIndex(imgPt, index))
      {
        continue;
      }
      value += static_cast<double>(this->m_FixedImage->GetPixel(index));
    }
    return static_cast<MeasureType>(value);
  }

  void
  GetDerivative(const ParametersType &, DerivativeType & derivative) const override
  {
    derivative.SetSize(this->GetNumberOfParameters());
    derivative.Fill(0);
  }

  void
  GetValueAndDerivative(const ParametersType & parameters,
                        MeasureType &          value,
                        DerivativeType &       derivative) const override
  {
    value = this->GetValue(parameters);
    this->GetDerivative(parameters, derivative);
  }
};

template <typename TPixel>
static void
RunSpatialObjectReg(int)
{
  using Img = itk::Image<TPixel, 2>;
  using Ellipse = itk::EllipseSpatialObject<2>;
  using Group = itk::GroupSpatialObject<2>;
  auto ellipse = Ellipse::New();
  ellipse->SetRadiusInObjectSpace(8);
  typename Ellipse::PointType c;
  c[0] = 32;
  c[1] = 32;
  ellipse->SetCenterInObjectSpace(c);
  ellipse->Update();
  auto group = Group::New();
  group->AddChild(ellipse);
  group->Update();
  auto raster = itk::SpatialObjectToImageFilter<Group, Img>::New();
  raster->SetInput(group);
  typename Img::SizeType sz;
  sz.Fill(64);
  raster->SetSize(sz);
  raster->Update();
  auto blur = itk::DiscreteGaussianImageFilter<Img, Img>::New();
  blur->SetInput(raster->GetOutput());
  blur->SetVariance(4.0);
  blur->Update();
  auto image = Hold(blur->GetOutput());

  using Reg = itk::ImageToSpatialObjectRegistrationMethod<Img, Group>;
  using Metric = SimpleSOMetric<Img, Group>;
  using Trans = itk::Euler2DTransform<double>;
  auto reg = Reg::New();
  auto metric = Metric::New();
  auto trans = Trans::New();
  auto interp = itk::LinearInterpolateImageFunction<Img, double>::New();
  auto opt = itk::OnePlusOneEvolutionaryOptimizer::New();
  metric->SetTransform(trans);
  typename Trans::ParametersType scales;
  scales.SetSize(3);
  scales[0] = 100;
  scales[1] = 1;
  scales[2] = 1;
  opt->SetScales(scales);
  typename Trans::ParametersType init;
  init.SetSize(3);
  init[0] = 0.05;
  init[1] = 1.0;
  init[2] = 1.0;
  auto gen = itk::Statistics::NormalVariateGenerator::New();
  gen->Initialize(12345);
  opt->SetNormalVariateGenerator(gen);
  opt->Initialize(1.02, 1.1);
  opt->SetEpsilon(0.05);
  opt->SetMaximumIteration(8);
  opt->MaximizeOn();
  reg->SetFixedImage(image);
  reg->SetMovingSpatialObject(group);
  reg->SetMetric(metric);
  reg->SetTransform(trans);
  reg->SetInterpolator(interp);
  reg->SetOptimizer(opt);
  reg->SetInitialTransformParameters(init);
  reg->Update();
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
  if (!g_list)
  {
    std::cerr << "cat3 construct threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << std::endl;
  }

  BenchTime("QuadEdgeMeshFiltering", "BorderQuadEdgeMeshFilter", []() { RunBorder<float>(0); },
            []() { RunBorder<double>(0); }, runs);
  BenchTime("QuadEdgeMeshFiltering", "ParameterizationQuadEdgeMeshFilter", []() { RunParameterization<double>(0); },
            []() { RunParameterization<double>(0); }, runs);
  BenchTime("QuadEdgeMeshFiltering", "NormalQuadEdgeMeshFilter", []() { RunNormal<float>(0); },
            []() { RunNormal<double>(0); }, runs);
  BenchTime("QuadEdgeMeshFiltering", "QuadricDecimationQuadEdgeMeshFilter", []() { RunQuadric<float>(0); },
            []() { RunQuadric<double>(0); }, runs);
  BenchTime("QuadEdgeMeshFiltering", "SquaredEdgeLengthDecimationQuadEdgeMeshFilter",
            []() { RunSquaredDecim<float>(0); }, []() { RunSquaredDecim<double>(0); }, runs);
  BenchTime("QuadEdgeMeshFiltering", "LaplacianDeformationQuadEdgeMeshFilterWithHardConstraints",
            []() { RunLapHard<double>(0); }, []() { RunLapHard<double>(0); }, runs);
  BenchTime("QuadEdgeMeshFiltering", "LaplacianDeformationQuadEdgeMeshFilterWithSoftConstraints",
            []() { RunLapSoft<double>(0); }, []() { RunLapSoft<double>(0); }, runs);
  BenchTime("Path", "OrthogonalSwath2DPathFilter", []() { RunOrthogonalSwath<float>(0); },
            []() { RunOrthogonalSwath<double>(0); }, runs);
  BenchTime("LevelSetsv4", "LevelSetDomainMapImageFilter", []() { RunDomainMap(0); }, []() { RunDomainMap(0); }, runs);
  BenchTime("Metricsv4", "LabeledPointSetToPointSetMetricv4", []() { RunLabeledPointSet(0); },
            []() { RunLabeledPointSet(0); }, runs);
  BenchTime("LevelSets", "GeodesicActiveContourShapePriorLevelSetImageFilter", []() { RunShapePrior<float>(0); },
            []() { RunShapePrior<double>(0); }, runs);
  BenchTime("DeformableMesh", "DeformableSimplexMesh3DFilter", []() { RunSimplexBase<float>(0); },
            []() { RunSimplexBase<double>(0); }, runs);
  BenchTime("DeformableMesh", "DeformableSimplexMesh3DBalloonForceFilter", []() { RunSimplexBalloon<float>(0); },
            []() { RunSimplexBalloon<double>(0); }, runs);
  BenchTime("DeformableMesh", "DeformableSimplexMesh3DGradientConstraintForceFilter",
            []() { RunSimplexGrad<float>(0); }, []() { RunSimplexGrad<double>(0); }, runs);
  BenchTime("Common", "ImageToSpatialObjectRegistrationMethod", []() { RunSpatialObjectReg<float>(0); },
            []() { RunSpatialObjectReg<double>(0); }, runs);
  return 0;
}
