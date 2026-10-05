// Instantiable leftover classes from the CSV 未测 list:
// ImageRegistrationMethodv4, MultiResolution, SyN, BSplineSyN,
// TimeVaryingVelocityField, TimeVaryingBSplineVelocityField,
// DiscreteCurvatureTensorQuadEdgeMeshFilter, MRFImageFilter, RGBGibbsPriorFilter.
// precision_remainder_instantiable_bench --list
// precision_remainder_instantiable_bench <image.png> [runs=1] [OperatorName]
#include "itkBSplineSmoothingOnUpdateDisplacementFieldTransform.h"
#include "itkBSplineSyNImageRegistrationMethod.h"
#include "itkCastImageFilter.h"
#include "itkComposeImageFilter.h"
#include "itkDiscreteCurvatureTensorQuadEdgeMeshFilter.h"
#include "itkDisplacementFieldTransform.h"
#include "itkImage.h"
#include "itkImageClassifierBase.h"
#include "itkImageFileReader.h"
#include "itkImageGaussianModelEstimator.h"
#include "itkImageRegistrationMethodv4.h"
#include "itkImageRegionIterator.h"
#include "itkImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMRFImageFilter.h"
#include "itkMahalanobisDistanceMembershipFunction.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkMinimumDecisionRule.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkMultiThreaderBase.h"
#include "itkPNGImageIOFactory.h"
#include "itkQuadEdgeMesh.h"
#include "itkRGBGibbsPriorFilter.h"
#include "itkRecursiveMultiResolutionPyramidImageFilter.h"
#include "itkRegularSphereMeshSource.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"
#include "itkResampleImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "itkSyNImageRegistrationMethod.h"
#include "itkTimeVaryingBSplineVelocityFieldImageRegistrationMethod.h"
#include "itkTimeVaryingBSplineVelocityFieldTransform.h"
#include "itkGaussianSmoothingOnUpdateTimeVaryingVelocityFieldTransform.h"
#include "itkTimeVaryingVelocityFieldImageRegistrationMethodv4.h"
#include "itkTimeVaryingVelocityFieldTransform.h"
#include "itkTimeVaryingVelocityFieldTransformParametersAdaptor.h"
#include "itkTranslationTransform.h"
#include "itkVector.h"

#include "precision_mode.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;

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
Emit(const std::string & module, const std::string & op, double msF, double msD, double err)
{
  const double sp = msF > 0.0 ? (msD / msF) : 0.0;
  std::cout << std::fixed << std::setprecision(8) << module << ',' << op << ',' << msF << ',' << msD << ',' << sp << ','
            << err << ",0" << std::endl;
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
    double msF = 0.0;
    double msD = 0.0;
    TimePrec(runF, runD, runs, [](auto fn, int n) { return TimeRuns(fn, n); }, msF, msD);
    Emit(module, op, msF, msD, 0.0);
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL," << e.GetDescription() << ",\n";
  }
  catch (const std::exception & e)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL," << e.what() << ",\n";
  }
}

template <typename TImage>
static typename TImage::Pointer
Resize(const FImg * in, unsigned int n)
{
  using Out = TImage;
  using Cast = itk::CastImageFilter<FImg, Out>;
  auto c = Cast::New();
  c->SetInput(in);
  c->Update();
  using Res = itk::ResampleImageFilter<Out, Out>;
  auto r = Res::New();
  typename Out::SizeType sz;
  sz.Fill(n);
  r->SetInput(c->GetOutput());
  r->SetSize(sz);
  r->SetOutputOrigin(c->GetOutput()->GetOrigin());
  auto sp = c->GetOutput()->GetSpacing();
  const auto old = c->GetOutput()->GetLargestPossibleRegion().GetSize();
  for (unsigned int d = 0; d < Dim; ++d)
  {
    sp[d] *= static_cast<double>(old[d]) / static_cast<double>(n);
  }
  r->SetOutputSpacing(sp);
  r->SetOutputDirection(c->GetOutput()->GetDirection());
  r->SetDefaultPixelValue(0);
  r->Update();
  return Hold(r->GetOutput());
}

template <typename TImage>
static typename TImage::Pointer
Shift(const TImage * in, double dx)
{
  using Trans = itk::TranslationTransform<double, Dim>;
  using Res = itk::ResampleImageFilter<TImage, TImage>;
  auto t = Trans::New();
  typename Trans::OutputVectorType off;
  off[0] = dx;
  off[1] = 0.0;
  t->SetOffset(off);
  auto r = Res::New();
  r->SetInput(in);
  r->SetTransform(t);
  r->SetSize(in->GetLargestPossibleRegion().GetSize());
  r->SetOutputOrigin(in->GetOrigin());
  r->SetOutputSpacing(in->GetSpacing());
  r->SetOutputDirection(in->GetDirection());
  r->SetDefaultPixelValue(0);
  r->Update();
  return Hold(r->GetOutput());
}

template <typename PixelType>
static void
RunV4(const FImg * src, unsigned int n, unsigned int iters)
{
  using ImageType = itk::Image<PixelType, Dim>;
  using TransformType = itk::TranslationTransform<double, Dim>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
  using RegistrationType = itk::ImageRegistrationMethodv4<ImageType, ImageType, TransformType>;
  auto fixed = Resize<ImageType>(src, n);
  auto moving = Shift(fixed.GetPointer(), 2.0);
  auto metric = MetricType::New();
  auto optimizer = OptimizerType::New();
  auto registration = RegistrationType::New();
  using Interp = itk::LinearInterpolateImageFunction<ImageType, double>;
  auto fi = Interp::New();
  auto mi = Interp::New();
  metric->SetFixedInterpolator(fi);
  metric->SetMovingInterpolator(mi);
  optimizer->SetNumberOfIterations(iters);
  optimizer->SetLearningRate(1.0);
  optimizer->SetMinimumStepLength(0.01);
  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  auto init = TransformType::New();
  typename TransformType::ParametersType p(init->GetNumberOfParameters());
  p.Fill(0.0);
  init->SetParameters(p);
  registration->SetMovingInitialTransform(init);
  registration->SetNumberOfLevels(1);
  registration->Update();
}

template <typename PixelType>
static void
RunMultiRes(const FImg * src, unsigned int n, unsigned int iters)
{
  using ImageType = itk::Image<PixelType, Dim>;
  using TransformType = itk::TranslationTransform<double, Dim>;
  using MetricType = itk::MeanSquaresImageToImageMetric<ImageType, ImageType>;
  using OptimizerType = itk::RegularStepGradientDescentOptimizer;
  using Interp = itk::LinearInterpolateImageFunction<ImageType, double>;
  using Pyramid = itk::RecursiveMultiResolutionPyramidImageFilter<ImageType, ImageType>;
  using RegistrationType = itk::MultiResolutionImageRegistrationMethod<ImageType, ImageType>;
  auto fixed = Resize<ImageType>(src, n);
  auto moving = Shift(fixed.GetPointer(), 2.0);
  auto metric = MetricType::New();
  auto optimizer = OptimizerType::New();
  auto interpolator = Interp::New();
  auto transform = TransformType::New();
  auto registration = RegistrationType::New();
  optimizer->SetMaximumStepLength(1.0);
  optimizer->SetMinimumStepLength(0.01);
  optimizer->SetNumberOfIterations(iters);
  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetTransform(transform);
  registration->SetInterpolator(interpolator);
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetFixedImageRegion(fixed->GetBufferedRegion());
  registration->SetFixedImagePyramid(Pyramid::New());
  registration->SetMovingImagePyramid(Pyramid::New());
  registration->SetNumberOfLevels(2);
  typename TransformType::ParametersType p(transform->GetNumberOfParameters());
  p.Fill(0.0);
  registration->SetInitialTransformParameters(p);
  registration->Update();
}

template <typename PixelType>
static void
RunSyN(const FImg * src, unsigned int n, unsigned int iters)
{
  using ImageType = itk::Image<PixelType, Dim>;
  using TransformType = itk::DisplacementFieldTransform<double, Dim>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using RegistrationType = itk::SyNImageRegistrationMethod<ImageType, ImageType, TransformType>;
  auto fixed = Resize<ImageType>(src, n);
  auto moving = Shift(fixed.GetPointer(), 1.5);
  auto metric = MetricType::New();
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metric);
  registration->SetLearningRate(0.25);
  registration->SetNumberOfLevels(1);
  typename RegistrationType::ShrinkFactorsArrayType shrink;
  shrink.SetSize(1);
  shrink[0] = 2;
  registration->SetShrinkFactorsPerLevel(shrink);
  typename RegistrationType::SmoothingSigmasArrayType sig;
  sig.SetSize(1);
  sig[0] = 0;
  registration->SetSmoothingSigmasPerLevel(sig);
  typename RegistrationType::NumberOfIterationsArrayType niter;
  niter.SetSize(1);
  niter[0] = iters;
  registration->SetNumberOfIterationsPerLevel(niter);
  registration->Update();
}

template <typename PixelType>
static void
RunBSplineSyN(const FImg * src, unsigned int n, unsigned int iters)
{
  using ImageType = itk::Image<PixelType, Dim>;
  using TransformType = itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<double, Dim>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using RegistrationType = itk::BSplineSyNImageRegistrationMethod<ImageType, ImageType, TransformType>;
  auto fixed = Resize<ImageType>(src, n);
  auto moving = Shift(fixed.GetPointer(), 1.5);
  auto metric = MetricType::New();
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metric);
  registration->SetLearningRate(0.25);
  registration->SetNumberOfLevels(1);
  typename RegistrationType::ShrinkFactorsArrayType shrink;
  shrink.SetSize(1);
  shrink[0] = 2;
  registration->SetShrinkFactorsPerLevel(shrink);
  typename RegistrationType::SmoothingSigmasArrayType sig;
  sig.SetSize(1);
  sig[0] = 0;
  registration->SetSmoothingSigmasPerLevel(sig);
  typename RegistrationType::NumberOfIterationsArrayType niter;
  niter.SetSize(1);
  niter[0] = iters;
  registration->SetNumberOfIterationsPerLevel(niter);
  registration->Update();
}

template <typename PixelType>
static void
RunTV(const FImg * src, unsigned int n, unsigned int iters)
{
  using ImageType = itk::Image<PixelType, Dim>;
  using RealType = double;
  using VectorType = itk::Vector<RealType, Dim>;
  using VelocityFieldType = itk::Image<VectorType, Dim + 1>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using RegistrationType = itk::TimeVaryingVelocityFieldImageRegistrationMethodv4<ImageType, ImageType>;
  auto fixed = Resize<ImageType>(src, n);
  auto moving = Shift(fixed.GetPointer(), 1.5);
  auto velocityField = VelocityFieldType::New();
  typename VelocityFieldType::SizeType vsz;
  vsz.Fill(4);
  const auto isz = fixed->GetLargestPossibleRegion().GetSize();
  vsz[0] = isz[0];
  vsz[1] = isz[1];
  vsz[2] = 2;
  typename VelocityFieldType::SpacingType vsp;
  vsp.Fill(1.0);
  vsp[0] = fixed->GetSpacing()[0];
  vsp[1] = fixed->GetSpacing()[1];
  vsp[2] = 1.0;
  typename VelocityFieldType::PointType vorg;
  vorg.Fill(0.0);
  vorg[0] = fixed->GetOrigin()[0];
  vorg[1] = fixed->GetOrigin()[1];
  vorg[2] = 0.0;
  velocityField->SetRegions(vsz);
  velocityField->SetSpacing(vsp);
  velocityField->SetOrigin(vorg);
  velocityField->Allocate();
  velocityField->FillBuffer(VectorType{});
  using OutputTransformType = itk::GaussianSmoothingOnUpdateTimeVaryingVelocityFieldTransform<double, Dim>;
  auto outputTransform = OutputTransformType::New();
  outputTransform->SetVelocityField(velocityField);
  outputTransform->SetLowerTimeBound(0.0);
  outputTransform->SetUpperTimeBound(1.0);
  outputTransform->SetGaussianSpatialSmoothingVarianceForTheUpdateField(3.0);
  outputTransform->SetGaussianTemporalSmoothingVarianceForTheUpdateField(0.5);
  outputTransform->IntegrateVelocityField();
  auto metric = MetricType::New();
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metric);
  registration->SetLearningRate(0.5);
  registration->SetInitialTransform(outputTransform);
  registration->InPlaceOn();
  registration->SetNumberOfLevels(1);
  typename RegistrationType::ShrinkFactorsArrayType shrink;
  shrink.SetSize(1);
  shrink[0] = 1;
  registration->SetShrinkFactorsPerLevel(shrink);
  typename RegistrationType::SmoothingSigmasArrayType sig;
  sig.SetSize(1);
  sig[0] = 0;
  registration->SetSmoothingSigmasPerLevel(sig);
  typename RegistrationType::NumberOfIterationsArrayType niter;
  niter.SetSize(1);
  niter[0] = iters;
  registration->SetNumberOfIterationsPerLevel(niter);
  using Adaptor = itk::TimeVaryingVelocityFieldTransformParametersAdaptor<OutputTransformType>;
  auto adaptor = Adaptor::New();
  adaptor->SetRequiredSize(vsz);
  adaptor->SetRequiredSpacing(vsp);
  adaptor->SetRequiredOrigin(vorg);
  adaptor->SetRequiredDirection(velocityField->GetDirection());
  typename RegistrationType::TransformParametersAdaptorsContainerType adaptors;
  adaptors.push_back(adaptor);
  registration->SetTransformParametersAdaptorsPerLevel(adaptors);
  registration->Update();
}

template <typename PixelType>
static void
RunTVBSpline(const FImg * src, unsigned int n, unsigned int iters)
{
  using ImageType = itk::Image<PixelType, Dim>;
  using TransformType = itk::TimeVaryingBSplineVelocityFieldTransform<double, Dim>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using RegistrationType =
    itk::TimeVaryingBSplineVelocityFieldImageRegistrationMethod<ImageType, ImageType, TransformType>;
  auto fixed = Resize<ImageType>(src, n);
  auto moving = Shift(fixed.GetPointer(), 1.5);
  auto metric = MetricType::New();
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metric);
  registration->SetLearningRate(0.5);
  registration->SetNumberOfLevels(1);
  typename RegistrationType::ShrinkFactorsArrayType shrink;
  shrink.SetSize(1);
  shrink[0] = 2;
  registration->SetShrinkFactorsPerLevel(shrink);
  typename RegistrationType::SmoothingSigmasArrayType sig;
  sig.SetSize(1);
  sig[0] = 0;
  registration->SetSmoothingSigmasPerLevel(sig);
  typename RegistrationType::NumberOfIterationsArrayType niter;
  niter.SetSize(1);
  niter[0] = iters;
  registration->SetNumberOfIterationsPerLevel(niter);
  registration->Update();
}

template <typename PixelType>
static void
RunCurvatureTensor()
{
  using Mesh = itk::QuadEdgeMesh<PixelType, 3>;
  auto src = itk::RegularSphereMeshSource<Mesh>::New();
  src->SetResolution(2);
  src->Update();
  auto f = itk::DiscreteCurvatureTensorQuadEdgeMeshFilter<Mesh, Mesh>::New();
  f->SetInput(src->GetOutput());
  f->Update();
}

template <typename Real>
static void
RunMRF()
{
  constexpr unsigned int bands = 2;
  constexpr unsigned int nclass = 3;
  using Vec = itk::Vector<Real, bands>;
  using VecImage = itk::Image<Vec, 2>;
  using ClassImage = itk::Image<unsigned char, 2>;
  auto vecImage = VecImage::New();
  auto classImage = ClassImage::New();
  typename VecImage::SizeType sz;
  sz.Fill(32);
  vecImage->SetRegions(sz);
  vecImage->Allocate();
  classImage->SetRegions(sz);
  classImage->Allocate();
  itk::ImageRegionIterator<VecImage> it(vecImage, vecImage->GetBufferedRegion());
  itk::ImageRegionIterator<ClassImage> cit(classImage, classImage->GetBufferedRegion());
  for (it.GoToBegin(), cit.GoToBegin(); !it.IsAtEnd(); ++it, ++cit)
  {
    const auto idx = it.GetIndex();
    Vec        v;
    const unsigned char lab = static_cast<unsigned char>((idx[0] < 16 ? 0 : 1) + (idx[1] < 16 ? 0 : 1));
    v[0] = static_cast<Real>(10 + 8 * lab);
    v[1] = static_cast<Real>(12 + 5 * lab);
    it.Set(v);
    cit.Set(lab > 2 ? 2 : lab);
  }
  using Membership = itk::Statistics::MahalanobisDistanceMembershipFunction<Vec>;
  using Estimator = itk::ImageGaussianModelEstimator<VecImage, Membership, ClassImage>;
  auto est = Estimator::New();
  est->SetNumberOfModels(nclass);
  est->SetInputImage(vecImage);
  est->SetTrainingImage(classImage);
  est->Update();
  using Decision = itk::Statistics::MinimumDecisionRule;
  auto decision = Decision::New();
  using Classifier = itk::ImageClassifierBase<VecImage, ClassImage>;
  auto classifier = Classifier::New();
  classifier->SetNumberOfClasses(nclass);
  classifier->SetDecisionRule(decision);
  auto fns = est->GetMembershipFunctions();
  for (unsigned int i = 0; i < nclass; ++i)
  {
    classifier->AddMembershipFunction(fns[i]);
  }
  const unsigned int oldThreads = itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads();
  itk::MultiThreaderBase::SetGlobalDefaultNumberOfThreads(1);
  auto mrf = itk::MRFImageFilter<VecImage, ClassImage>::New();
  mrf->SetNumberOfClasses(nclass);
  mrf->SetMaximumNumberOfIterations(4);
  mrf->SetErrorTolerance(0.10);
  mrf->SetSmoothingFactor(1);
  mrf->SetNeighborhoodRadius(1);
  mrf->SetInput(vecImage);
  mrf->SetClassifier(classifier);
  mrf->Update();
  itk::MultiThreaderBase::SetGlobalDefaultNumberOfThreads(oldThreads);
}

template <typename Real>
static void
RunRGBGibbs()
{
  constexpr unsigned int bands = 3;
  constexpr unsigned int nclass = 2;
  using Vec = itk::Vector<Real, bands>;
  using VecImage = itk::Image<Vec, 3>;
  using ClassImage = itk::Image<unsigned char, 3>;
  auto vecImage = VecImage::New();
  auto classImage = ClassImage::New();
  typename VecImage::SizeType sz;
  sz[0] = 12;
  sz[1] = 12;
  sz[2] = 8;
  vecImage->SetRegions(sz);
  vecImage->Allocate();
  classImage->SetRegions(sz);
  classImage->Allocate();
  itk::ImageRegionIterator<VecImage> it(vecImage, vecImage->GetBufferedRegion());
  itk::ImageRegionIterator<ClassImage> cit(classImage, classImage->GetBufferedRegion());
  for (it.GoToBegin(), cit.GoToBegin(); !it.IsAtEnd(); ++it, ++cit)
  {
    const auto idx = it.GetIndex();
    const unsigned char lab = idx[0] < 6 ? 0 : 1;
    Vec                v;
    v[0] = static_cast<Real>(10 + 80 * lab);
    v[1] = static_cast<Real>(12 + 70 * lab);
    v[2] = static_cast<Real>(8 + 60 * lab);
    it.Set(v);
    cit.Set(lab);
  }
  using Membership = itk::Statistics::MahalanobisDistanceMembershipFunction<Vec>;
  using Estimator = itk::ImageGaussianModelEstimator<VecImage, Membership, ClassImage>;
  auto est = Estimator::New();
  est->SetNumberOfModels(nclass);
  est->SetInputImage(vecImage);
  est->SetTrainingImage(classImage);
  est->Update();
  using Decision = itk::Statistics::MinimumDecisionRule;
  auto decision = Decision::New();
  using Classifier = itk::ImageClassifierBase<VecImage, ClassImage>;
  auto classifier = Classifier::New();
  classifier->SetNumberOfClasses(nclass);
  classifier->SetDecisionRule(decision);
  auto fns = est->GetMembershipFunctions();
  for (unsigned int i = 0; i < nclass; ++i)
  {
    classifier->AddMembershipFunction(fns[i]);
  }
  auto f = itk::RGBGibbsPriorFilter<VecImage, ClassImage>::New();
  f->SetInput(vecImage);
  f->SetTrainingImage(classImage);
  f->SetClassifier(classifier);
  f->SetNumberOfClasses(nclass);
  f->SetMaximumNumberOfIterations(3);
  f->SetObjectLabel(1);
  typename ClassImage::IndexType start;
  start.Fill(2);
  f->SetStartPoint(start);
  f->Update();
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
    std::cerr << "usage: " << argv[0] << " --list | <image.png> [runs=1] [OperatorName]\n";
    return 1;
  }
  itk::PNGImageIOFactory::RegisterOneFactory();
  const int runs = (!g_list && argc >= 3) ? std::stoi(argv[2]) : 1;
  if (!g_list && argc >= 4)
  {
    g_only = argv[3];
  }
  FImg::Pointer input;
  if (!g_list)
  {
    using Reader = itk::ImageFileReader<FImg>;
    auto r = Reader::New();
    r->SetFileName(argv[1]);
    r->Update();
    input = r->GetOutput();
  }
  if (!g_list)
  {
    std::cout << "module,operator,ms_float,ms_double,speedup,max_abs,rmse\n";
  }
  const FImg * src = input.GetPointer();
  BenchTime(
    "RegistrationMethodsv4",
    "ImageRegistrationMethodv4",
    [&]() { RunV4<float>(src, 128, 8); },
    [&]() { RunV4<double>(src, 128, 8); },
    runs);
  BenchTime(
    "Registration/Common",
    "MultiResolutionImageRegistrationMethod",
    [&]() { RunMultiRes<float>(src, 128, 8); },
    [&]() { RunMultiRes<double>(src, 128, 8); },
    runs);
  BenchTime(
    "RegistrationMethodsv4",
    "SyNImageRegistrationMethod",
    [&]() { RunSyN<float>(src, 64, 2); },
    [&]() { RunSyN<double>(src, 64, 2); },
    runs);
  BenchTime(
    "RegistrationMethodsv4",
    "BSplineSyNImageRegistrationMethod",
    [&]() { RunBSplineSyN<float>(src, 64, 2); },
    [&]() { RunBSplineSyN<double>(src, 64, 2); },
    runs);
  BenchTime(
    "RegistrationMethodsv4",
    "TimeVaryingVelocityFieldImageRegistrationMethodv4",
    [&]() { RunTV<float>(src, 32, 2); },
    [&]() { RunTV<double>(src, 32, 2); },
    runs);
  BenchTime(
    "RegistrationMethodsv4",
    "TimeVaryingBSplineVelocityFieldImageRegistrationMethod",
    [&]() { RunTVBSpline<float>(src, 32, 2); },
    [&]() { RunTVBSpline<double>(src, 32, 2); },
    runs);
  BenchTime(
    "QuadEdgeMeshFiltering",
    "DiscreteCurvatureTensorQuadEdgeMeshFilter",
    [&]() { RunCurvatureTensor<float>(); },
    [&]() { RunCurvatureTensor<double>(); },
    runs);
  BenchTime(
    "MarkovRandomFieldsClassifiers",
    "MRFImageFilter",
    [&]() { RunMRF<float>(); },
    [&]() { RunMRF<double>(); },
    runs);
  BenchTime(
    "MarkovRandomFieldsClassifiers",
    "RGBGibbsPriorFilter",
    [&]() { RunRGBGibbs<float>(); },
    [&]() { RunRGBGibbs<double>(); },
    runs);
  return 0;
}
