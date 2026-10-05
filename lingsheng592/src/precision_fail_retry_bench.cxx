// Retry the 6 CSV failures with smaller or official-test inputs.
// CurvatureRegistrationFilter needs FFTW; this install has it OFF.
// precision_fail_retry_bench --list | <image.png> [runs=1] [OperatorName]
#include "itkBinaryThinningImageFilter.h"
#include "itkBSplineSmoothingOnUpdateDisplacementFieldTransform.h"
#include "itkBSplineSyNImageRegistrationMethod.h"
#include "itkCastImageFilter.h"
#include "itkHoughTransform2DLinesImageFilter.h"
#include "itkImage.h"
#include "itkImageClassifierBase.h"
#include "itkImageFileReader.h"
#include "itkImageGaussianModelEstimator.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMahalanobisDistanceMembershipFunction.h"
#include "itkMRFImageFilter.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkMinimumDecisionRule.h"
#include "itkMultiThreaderBase.h"
#include "itkPNGImageIOFactory.h"
#include "itkRGBGibbsPriorFilter.h"
#include "itkResampleImageFilter.h"
#include "itkSyNImageRegistrationMethod.h"
#include "itkTimeVaryingBSplineVelocityFieldImageRegistrationMethod.h"
#include "itkTimeVaryingBSplineVelocityFieldTransform.h"
#include "itkTranslationTransform.h"
#include "itkVector.h"

#include "precision_mode.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;

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
  std::cout << std::fixed << std::setprecision(8) << module << ',' << op << ',' << msF << ',' << msD << ',' << sp
            << ",0,0\n";
}

template <typename FnF, typename FnD>
static void
BenchTime(const std::string & module, const std::string & op, FnF runF, FnD runD, int runs)
{
  if (!Want(op))
  {
    return;
  }
  std::cerr << ">> " << op << std::endl;
  try
  {
    double msF = 0.0;
    double msD = 0.0;
    TimePrec(runF, runD, runs, [](auto fn, int n) { return TimeRuns(fn, n); }, msF, msD);
    Emit(module, op, msF, msD);
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL," << e.GetDescription() << ",\n";
  }
}

template <typename TImage>
static typename TImage::Pointer
MakeBox(unsigned n, typename TImage::PixelType fg)
{
  auto img = TImage::New();
  typename TImage::SizeType sz;
  sz.Fill(n);
  img->SetRegions(sz);
  img->Allocate();
  img->FillBuffer(0);
  itk::ImageRegionIteratorWithIndex<TImage> it(img, img->GetBufferedRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto x = it.GetIndex()[0];
    const auto y = it.GetIndex()[1];
    if ((x > n / 4 && x < 3 * n / 4 && y > n / 2 - 2 && y < n / 2 + 2) ||
        (y > n / 4 && y < 3 * n / 4 && x > n / 2 - 2 && x < n / 2 + 2))
    {
      it.Set(fg);
    }
  }
  return img;
}

template <typename TImage>
static typename TImage::Pointer
MakeLine(unsigned n)
{
  auto img = TImage::New();
  typename TImage::SizeType sz;
  sz.Fill(n);
  img->SetRegions(sz);
  img->Allocate();
  img->FillBuffer(0);
  const unsigned y = n / 2;
  for (unsigned x = n / 8; x < 7 * n / 8; ++x)
  {
    typename TImage::IndexType idx;
    idx[0] = static_cast<long>(x);
    idx[1] = static_cast<long>(y);
    img->SetPixel(idx, 1);
  }
  return img;
}

template <typename TImage>
static typename TImage::Pointer
Resize(const FImg * in, unsigned n)
{
  using Cast = itk::CastImageFilter<FImg, TImage>;
  auto c = Cast::New();
  c->SetInput(in);
  c->Update();
  using Res = itk::ResampleImageFilter<TImage, TImage>;
  auto r = Res::New();
  typename TImage::SizeType sz;
  sz.Fill(n);
  auto sp = c->GetOutput()->GetSpacing();
  const auto old = c->GetOutput()->GetLargestPossibleRegion().GetSize();
  for (unsigned d = 0; d < Dim; ++d)
  {
    sp[d] *= static_cast<double>(old[d]) / static_cast<double>(n);
  }
  r->SetInput(c->GetOutput());
  r->SetSize(sz);
  r->SetOutputOrigin(c->GetOutput()->GetOrigin());
  r->SetOutputSpacing(sp);
  r->SetOutputDirection(c->GetOutput()->GetDirection());
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
  off[1] = 0;
  t->SetOffset(off);
  auto r = Res::New();
  r->SetInput(in);
  r->SetTransform(t);
  r->SetSize(in->GetLargestPossibleRegion().GetSize());
  r->SetOutputOrigin(in->GetOrigin());
  r->SetOutputSpacing(in->GetSpacing());
  r->SetOutputDirection(in->GetDirection());
  r->Update();
  return Hold(r->GetOutput());
}

template <typename PixelType>
static void
RunThin()
{
  using Img = itk::Image<PixelType, 2>;
  auto in = MakeBox<Img>(64, 1);
  auto f = itk::BinaryThinningImageFilter<Img, Img>::New();
  f->SetInput(in);
  f->Update();
}

template <typename PixelType>
static void
RunHough()
{
  using Img = itk::Image<PixelType, 2>;
  auto in = MakeLine<Img>(64);
  auto f = itk::HoughTransform2DLinesImageFilter<PixelType, PixelType>::New();
  f->SetInput(in);
  f->SetNumberOfLines(1);
  f->SetVariance(1.0);
  f->Update();
}

template <typename PixelType>
static void
RunBSplineSyN(const FImg * src)
{
  using ImageType = itk::Image<PixelType, 2>;
  using TransformType = itk::BSplineSmoothingOnUpdateDisplacementFieldTransform<double, 2>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using RegistrationType = itk::BSplineSyNImageRegistrationMethod<ImageType, ImageType, TransformType>;
  auto fixed = Resize<ImageType>(src, 32);
  auto moving = Shift(fixed.GetPointer(), 1.0);
  auto transform = TransformType::New();
  transform->SetSplineOrder(3);
  typename TransformType::ArrayType cp;
  cp.Fill(6);
  transform->SetNumberOfControlPointsForTheUpdateField(cp);
  transform->SetNumberOfControlPointsForTheTotalField(cp);
  using Field = typename TransformType::DisplacementFieldType;
  auto field = Field::New();
  field->CopyInformation(fixed);
  field->SetRegions(fixed->GetLargestPossibleRegion());
  field->Allocate();
  field->FillBuffer(typename Field::PixelType{});
  transform->SetDisplacementField(field);
  auto metric = MetricType::New();
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metric);
  registration->SetInitialTransform(transform);
  registration->InPlaceOn();
  registration->SetLearningRate(0.25);
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
  niter[0] = 1;
  registration->SetNumberOfIterationsPerLevel(niter);
  registration->Update();
}

template <typename PixelType>
static void
RunTVBSpline(const FImg * src)
{
  using ImageType = itk::Image<PixelType, 2>;
  using TransformType = itk::TimeVaryingBSplineVelocityFieldTransform<double, 2>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using RegistrationType =
    itk::TimeVaryingBSplineVelocityFieldImageRegistrationMethod<ImageType, ImageType, TransformType>;
  auto fixed = Resize<ImageType>(src, 16);
  auto moving = Shift(fixed.GetPointer(), 1.0);
  auto transform = TransformType::New();
  auto metric = MetricType::New();
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metric);
  registration->SetInitialTransform(transform);
  registration->SetLearningRate(0.25);
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
  niter[0] = 1;
  registration->SetNumberOfIterationsPerLevel(niter);
  registration->Update();
}

template <typename Real>
static void
RunMRF()
{
  itk::MultiThreaderBase::SetGlobalDefaultNumberOfThreads(1);
  constexpr unsigned int bands = 2;
  constexpr unsigned int nclass = 3;
  using Vec = itk::Vector<Real, bands>;
  using VecImage = itk::Image<Vec, 3>;
  using ClassImage = itk::Image<unsigned char, 3>;
  typename VecImage::SizeType sz;
  sz[0] = 6;
  sz[1] = 6;
  sz[2] = 3;
  auto vecImage = VecImage::New();
  auto classImage = ClassImage::New();
  vecImage->SetRegions(sz);
  vecImage->Allocate();
  classImage->SetRegions(sz);
  classImage->Allocate();
  itk::ImageRegionIteratorWithIndex<VecImage> it(vecImage, vecImage->GetBufferedRegion());
  itk::ImageRegionIteratorWithIndex<ClassImage> cit(classImage, classImage->GetBufferedRegion());
  for (it.GoToBegin(), cit.GoToBegin(); !it.IsAtEnd(); ++it, ++cit)
  {
    const auto z = it.GetIndex()[2];
    const auto y = it.GetIndex()[1];
    unsigned char lab = 0;
    if (z == 0)
    {
      lab = y < 3 ? 2 : 1;
    }
    else if (z == 1)
    {
      lab = 0;
    }
    else
    {
      lab = y < 3 ? 2 : 1;
    }
    Vec v;
    v[0] = static_cast<Real>(10 + 8 * lab);
    v[1] = static_cast<Real>(12 + 5 * lab);
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
  auto decision = itk::Statistics::MinimumDecisionRule::New();
  using Classifier = itk::ImageClassifierBase<VecImage, ClassImage>;
  auto classifier = Classifier::New();
  classifier->SetNumberOfClasses(nclass);
  classifier->SetDecisionRule(decision);
  auto fns = est->GetMembershipFunctions();
  for (unsigned i = 0; i < nclass; ++i)
  {
    classifier->AddMembershipFunction(fns[i]);
  }
  auto mrf = itk::MRFImageFilter<VecImage, ClassImage>::New();
  mrf->SetNumberOfClasses(nclass);
  mrf->SetMaximumNumberOfIterations(3);
  mrf->SetErrorTolerance(0.10);
  mrf->SetSmoothingFactor(1);
  mrf->SetNeighborhoodRadius(1);
  mrf->SetInput(vecImage);
  mrf->SetClassifier(classifier);
  mrf->Update();
}

template <typename Real>
static void
RunRGBGibbs()
{
  itk::MultiThreaderBase::SetGlobalDefaultNumberOfThreads(1);
  using Vec = itk::Vector<Real, 1>;
  using VecImage = itk::Image<Vec, 3>;
  using ClassImage = itk::Image<unsigned char, 3>;
  typename VecImage::SizeType sz;
  sz[0] = 20;
  sz[1] = 20;
  sz[2] = 1;
  auto vecImage = VecImage::New();
  auto classImage = ClassImage::New();
  vecImage->SetRegions(sz);
  vecImage->Allocate();
  classImage->SetRegions(sz);
  classImage->Allocate();
  itk::ImageRegionIteratorWithIndex<VecImage> it(vecImage, vecImage->GetBufferedRegion());
  itk::ImageRegionIteratorWithIndex<ClassImage> cit(classImage, classImage->GetBufferedRegion());
  for (it.GoToBegin(), cit.GoToBegin(); !it.IsAtEnd(); ++it, ++cit)
  {
    const auto x = it.GetIndex()[0];
    const auto y = it.GetIndex()[1];
    unsigned char lab = 0;
    if (x > 4 && x < 8 && y > 4 && y < 8)
    {
      lab = 1;
    }
    else if (x > 14 && x < 18 && y > 14 && y < 18)
    {
      lab = 2;
    }
    Vec v;
    v[0] = static_cast<Real>(280 + 40 * lab);
    it.Set(v);
    cit.Set(lab);
  }
  using Membership = itk::Statistics::MahalanobisDistanceMembershipFunction<Vec>;
  using Estimator = itk::ImageGaussianModelEstimator<VecImage, Membership, ClassImage>;
  auto est = Estimator::New();
  est->SetNumberOfModels(3);
  est->SetInputImage(vecImage);
  est->SetTrainingImage(classImage);
  est->Update();
  auto decision = itk::Statistics::MinimumDecisionRule::New();
  using Classifier = itk::ImageClassifierBase<VecImage, ClassImage>;
  auto classifier = Classifier::New();
  classifier->SetNumberOfClasses(3);
  classifier->SetDecisionRule(decision);
  auto fns = est->GetMembershipFunctions();
  for (unsigned i = 0; i < 3; ++i)
  {
    classifier->AddMembershipFunction(fns[i]);
  }
  auto f = itk::RGBGibbsPriorFilter<VecImage, ClassImage>::New();
  f->SetNumberOfClasses(3);
  f->SetMaximumNumberOfIterations(1);
  f->SetClusterSize(10);
  f->SetBoundaryGradient(6);
  f->SetObjectLabel(1);
  typename ClassImage::IndexType start;
  start.Fill(0);
  f->SetStartPoint(start);
  f->SetCliqueWeight_1(5);
  f->SetCliqueWeight_2(5);
  f->SetCliqueWeight_3(5);
  f->SetCliqueWeight_4(5);
  f->SetCliqueWeight_5(5);
  f->SetCliqueWeight_6(0);
  f->SetInput(vecImage);
  f->SetClassifier(classifier);
  f->SetTrainingImage(classImage);
  f->SetObjectThreshold(5.0);
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
  BenchTime("BinaryMathematicalMorphology", "BinaryThinningImageFilter", [&]() { RunThin<float>(); },
            [&]() { RunThin<double>(); }, runs);
  BenchTime("ImageFeature", "HoughTransform2DLinesImageFilter", [&]() { RunHough<float>(); },
            [&]() { RunHough<double>(); }, runs);
  BenchTime("RegistrationMethodsv4", "BSplineSyNImageRegistrationMethod", [&]() { RunBSplineSyN<float>(src); },
            [&]() { RunBSplineSyN<double>(src); }, runs);
  BenchTime("RegistrationMethodsv4", "TimeVaryingBSplineVelocityFieldImageRegistrationMethod",
            [&]() { RunTVBSpline<float>(src); }, [&]() { RunTVBSpline<double>(src); }, runs);
  BenchTime("MarkovRandomFieldsClassifiers", "MRFImageFilter", [&]() { RunMRF<float>(); }, [&]() { RunMRF<double>(); },
            runs);
  BenchTime("MarkovRandomFieldsClassifiers", "RGBGibbsPriorFilter", [&]() { RunRGBGibbs<float>(); },
            [&]() { RunRGBGibbs<double>(); }, runs);
  if (Want("CurvatureRegistrationFilter"))
  {
    std::cout << "PDEDeformable,CurvatureRegistrationFilter,FAIL,FAIL,FAIL,needs_FFTW,\n";
  }
  return 0;
}
