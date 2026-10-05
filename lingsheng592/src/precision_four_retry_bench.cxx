// Retry VectorNeighborhood, UnaryFrequency, BSplineSyN, TimeVaryingBSpline.
// Vector/frequency compare component (or complex) max_abs, not grayscale pixels.
// Registration is float-storage vs double-storage. Official adaptor setup.
// precision_four_retry_bench --list | <image.png> [runs=1] [OperatorName]
#include "itkANTSNeighborhoodCorrelationImageToImageMetricv4.h"
#include "itkBSplineSmoothingOnUpdateDisplacementFieldTransformParametersAdaptor.h"
#include "itkBSplineSyNImageRegistrationMethod.h"
#include "itkCastImageFilter.h"
#include "itkDerivativeOperator.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkPNGImageIOFactory.h"
#include "itkResampleImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "itkTimeVaryingBSplineVelocityFieldImageRegistrationMethod.h"
#include "itkTimeVaryingBSplineVelocityFieldTransformParametersAdaptor.h"
#include "itkTranslationTransform.h"
#include "itkUnaryFrequencyDomainFilter.h"
#include "itkVector.h"
#include "itkVectorNeighborhoodOperatorImageFilter.h"
#include "itkVnlForwardFFTImageFilter.h"

#include "precision_mode.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <exception>
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
Emit(const std::string & module, const std::string & op, double msF, double msD, double err)
{
  const double sp = msF > 0.0 ? (msD / msF) : 0.0;
  std::cout << std::scientific << std::setprecision(8) << module << ',' << op << ',' << msF << ',' << msD << ',' << sp
            << ',' << err << ",0\n";
}

template <typename FnF, typename FnD, typename ErrFn>
static void
BenchTime(const std::string & module, const std::string & op, FnF runF, FnD runD, int runs, ErrFn errFn)
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
    Emit(module, op, msF, msD, errFn());
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
  off.Fill(0);
  off[0] = dx;
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

template <typename T>
static typename itk::Image<itk::Vector<T, 2>, Dim>::Pointer
MakeVec(const FImg * src, unsigned n)
{
  auto sc = Resize<itk::Image<T, Dim>>(src, n);
  using V = itk::Vector<T, 2>;
  using VImg = itk::Image<V, Dim>;
  auto out = VImg::New();
  out->CopyInformation(sc);
  out->SetRegions(sc->GetLargestPossibleRegion());
  out->Allocate();
  itk::ImageRegionConstIterator<itk::Image<T, Dim>> it(sc, sc->GetBufferedRegion());
  itk::ImageRegionIterator<VImg>                    ot(out, out->GetBufferedRegion());
  for (it.GoToBegin(), ot.GoToBegin(); !it.IsAtEnd(); ++it, ++ot)
  {
    V v;
    v[0] = it.Get();
    v[1] = it.Get() * static_cast<T>(0.5);
    ot.Set(v);
  }
  return out;
}

template <typename T>
static void
RunVector(const FImg * src)
{
  using V = itk::Vector<T, 2>;
  using VImg = itk::Image<V, Dim>;
  auto in = MakeVec<T>(src, 64);
  itk::DerivativeOperator<T, Dim> oper;
  oper.SetOrder(1);
  oper.SetDirection(0);
  oper.CreateDirectional();
  auto f = itk::VectorNeighborhoodOperatorImageFilter<VImg, VImg>::New();
  f->SetOperator(oper);
  f->SetInput(in);
  f->Update();
}

template <typename T>
static typename itk::Image<itk::Vector<T, 2>, Dim>::Pointer
RunVectorOut(const FImg * src)
{
  using V = itk::Vector<T, 2>;
  using VImg = itk::Image<V, Dim>;
  auto in = MakeVec<T>(src, 64);
  itk::DerivativeOperator<T, Dim> oper;
  oper.SetOrder(1);
  oper.SetDirection(0);
  oper.CreateDirectional();
  auto f = itk::VectorNeighborhoodOperatorImageFilter<VImg, VImg>::New();
  f->SetOperator(oper);
  f->SetInput(in);
  f->Update();
  return Hold(f->GetOutput());
}

static double
VecMaxAbs(const FImg * src)
{
  auto a = RunVectorOut<float>(src);
  auto b = RunVectorOut<double>(src);
  double m = 0.0;
  itk::ImageRegionConstIterator<itk::Image<itk::Vector<float, 2>, Dim>> ia(a, a->GetBufferedRegion());
  itk::ImageRegionConstIterator<itk::Image<itk::Vector<double, 2>, Dim>> ib(b, b->GetBufferedRegion());
  for (ia.GoToBegin(), ib.GoToBegin(); !ia.IsAtEnd(); ++ia, ++ib)
  {
    for (unsigned c = 0; c < 2; ++c)
    {
      m = std::max(m, std::abs(static_cast<double>(ia.Get()[c]) - ib.Get()[c]));
    }
  }
  return m;
}

template <typename T>
static typename itk::Image<std::complex<T>, Dim>::Pointer
ToFreq(const FImg * src, unsigned n)
{
  auto sc = Resize<itk::Image<T, Dim>>(src, n);
  using FFT = itk::VnlForwardFFTImageFilter<itk::Image<T, Dim>>;
  auto fft = FFT::New();
  fft->SetInput(sc);
  fft->Update();
  return Hold(fft->GetOutput());
}

template <typename T>
static void
RunFreq(typename itk::Image<std::complex<T>, Dim>::Pointer freq)
{
  using CImg = itk::Image<std::complex<T>, Dim>;
  auto f = itk::UnaryFrequencyDomainFilter<CImg>::New();
  f->SetInput(freq);
  f->SetFunctor([](const typename itk::UnaryFrequencyDomainFilter<CImg>::FrequencyIteratorType &) { return 1.0; });
  f->Update();
}

template <typename T>
static typename itk::Image<std::complex<T>, Dim>::Pointer
RunFreqOut(typename itk::Image<std::complex<T>, Dim>::Pointer freq)
{
  using CImg = itk::Image<std::complex<T>, Dim>;
  auto f = itk::UnaryFrequencyDomainFilter<CImg>::New();
  f->SetInput(freq);
  f->SetFunctor([](const typename itk::UnaryFrequencyDomainFilter<CImg>::FrequencyIteratorType &) { return 1.0; });
  f->Update();
  return Hold(f->GetOutput());
}

static double
FreqMaxAbs(typename itk::Image<std::complex<float>, Dim>::Pointer aIn,
           typename itk::Image<std::complex<double>, Dim>::Pointer bIn)
{
  auto a = RunFreqOut<float>(aIn);
  auto b = RunFreqOut<double>(bIn);
  double m = 0.0;
  itk::ImageRegionConstIterator<itk::Image<std::complex<float>, Dim>> ia(a, a->GetBufferedRegion());
  itk::ImageRegionConstIterator<itk::Image<std::complex<double>, Dim>> ib(b, b->GetBufferedRegion());
  for (ia.GoToBegin(), ib.GoToBegin(); !ia.IsAtEnd(); ++ia, ++ib)
  {
    const auto ca = ia.Get();
    const auto cb = ib.Get();
    m = std::max(m, std::abs(static_cast<double>(ca.real()) - cb.real()));
    m = std::max(m, std::abs(static_cast<double>(ca.imag()) - cb.imag()));
  }
  return m;
}

template <typename RegistrationType>
static void
SetOneLevel(RegistrationType * registration, unsigned int iterations, unsigned int shrink, double sigma)
{
  registration->SetNumberOfLevels(1);
  typename RegistrationType::ShrinkFactorsArrayType shrinkFactors(1);
  shrinkFactors[0] = shrink;
  registration->SetShrinkFactorsPerLevel(shrinkFactors);
  typename RegistrationType::SmoothingSigmasArrayType sigmas(1);
  sigmas[0] = static_cast<typename RegistrationType::SmoothingSigmasArrayType::ValueType>(sigma);
  registration->SetSmoothingSigmasPerLevel(sigmas);
  registration->SetSmoothingSigmasAreSpecifiedInPhysicalUnits(true);
  typename RegistrationType::NumberOfIterationsArrayType iters(1);
  iters[0] = iterations;
  registration->SetNumberOfIterationsPerLevel(iters);
}

template <typename Pixel>
static double
RunBSplineSyN(const FImg * src)
{
  using ImageType = itk::Image<Pixel, Dim>;
  using RegistrationType = itk::BSplineSyNImageRegistrationMethod<ImageType, ImageType>;
  using MetricType = itk::ANTSNeighborhoodCorrelationImageToImageMetricv4<ImageType, ImageType>;
  using OutputTransformType = typename RegistrationType::OutputTransformType;
  using VectorType = itk::Vector<double, Dim>;
  using DisplacementFieldType = itk::Image<VectorType, Dim>;
  using AdaptorType =
    itk::BSplineSmoothingOnUpdateDisplacementFieldTransformParametersAdaptor<OutputTransformType>;

  auto fixed = Resize<ImageType>(src, 32);
  auto moving = Shift(fixed.GetPointer(), 1.0);
  auto metricObj = MetricType::New();
  typename MetricType::RadiusType radius;
  radius.Fill(2);
  metricObj->SetRadius(radius);
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metricObj);
  registration->SetLearningRate(0.2);
  SetOneLevel(registration.GetPointer(), 1, 1, 0.0);

  constexpr VectorType zeroVector{};
  auto                 displacementField = DisplacementFieldType::New();
  displacementField->CopyInformation(fixed);
  displacementField->SetRegions(fixed->GetBufferedRegion());
  displacementField->Allocate();
  displacementField->FillBuffer(zeroVector);
  auto inverseDisplacementField = DisplacementFieldType::New();
  inverseDisplacementField->CopyInformation(fixed);
  inverseDisplacementField->SetRegions(fixed->GetBufferedRegion());
  inverseDisplacementField->Allocate();
  inverseDisplacementField->FillBuffer(zeroVector);

  auto outputTransform = OutputTransformType::New();
  outputTransform->SetDisplacementField(displacementField);
  outputTransform->SetInverseDisplacementField(inverseDisplacementField);

  using ShrinkFilterType = itk::ShrinkImageFilter<DisplacementFieldType, DisplacementFieldType>;
  auto shrinkFilter = ShrinkFilterType::New();
  shrinkFilter->SetShrinkFactors(1);
  shrinkFilter->SetInput(displacementField);
  shrinkFilter->Update();

  typename OutputTransformType::ArrayType updateMeshSize;
  typename OutputTransformType::ArrayType totalMeshSize;
  updateMeshSize.Fill(4);
  totalMeshSize.Fill(0);
  auto adaptor = AdaptorType::New();
  adaptor->SetRequiredSpacing(shrinkFilter->GetOutput()->GetSpacing());
  adaptor->SetRequiredSize(shrinkFilter->GetOutput()->GetBufferedRegion().GetSize());
  adaptor->SetRequiredDirection(shrinkFilter->GetOutput()->GetDirection());
  adaptor->SetRequiredOrigin(shrinkFilter->GetOutput()->GetOrigin());
  adaptor->SetTransform(outputTransform);
  adaptor->SetMeshSizeForTheUpdateField(updateMeshSize);
  adaptor->SetMeshSizeForTheTotalField(totalMeshSize);
  typename RegistrationType::TransformParametersAdaptorsContainerType adaptors;
  adaptors.push_back(adaptor.GetPointer());
  registration->SetTransformParametersAdaptorsPerLevel(adaptors);
  registration->SetInitialTransform(outputTransform);
  registration->InPlaceOn();
  registration->Update();
  return static_cast<double>(registration->GetMetric()->GetCurrentValue());
}

template <typename Pixel>
static double
RunTimeVaryingBSpline(const FImg * src)
{
  using ImageType = itk::Image<Pixel, Dim>;
  using RegistrationType = itk::TimeVaryingBSplineVelocityFieldImageRegistrationMethod<ImageType, ImageType>;
  using MetricType = itk::ANTSNeighborhoodCorrelationImageToImageMetricv4<ImageType, ImageType>;
  using OutputTransformType = typename RegistrationType::OutputTransformType;
  using AdaptorType = itk::TimeVaryingBSplineVelocityFieldTransformParametersAdaptor<OutputTransformType>;
  using VectorType = itk::Vector<typename OutputTransformType::ScalarType, Dim>;
  using LatticeType = itk::Image<VectorType, Dim + 1>;

  auto fixed = Resize<ImageType>(src, 32);
  auto moving = Shift(fixed.GetPointer(), 1.0);
  auto metricObj = MetricType::New();
  typename MetricType::RadiusType radius;
  radius.Fill(2);
  metricObj->SetRadius(radius);
  auto registration = RegistrationType::New();
  registration->SetFixedImage(fixed);
  registration->SetMovingImage(moving);
  registration->SetMetric(metricObj);
  registration->SetLearningRate(0.5);
  registration->SetNumberOfTimePointSamples(2);
  SetOneLevel(registration.GetPointer(), 1, 1, 0.0);

  auto output = OutputTransformType::New();
  output->SetSplineOrder(3);
  output->SetLowerTimeBound(0.0);
  output->SetUpperTimeBound(1.0);

  using ShrinkFilterType = itk::ShrinkImageFilter<ImageType, ImageType>;
  auto shrink = ShrinkFilterType::New();
  shrink->SetInput(fixed);
  shrink->SetShrinkFactors(1);
  shrink->Update();
  const auto * domain = shrink->GetOutput();

  typename LatticeType::PointType     domainOrigin;
  typename LatticeType::SpacingType   domainSpacing;
  typename LatticeType::SizeType      domainSize;
  typename LatticeType::SizeType      meshSize;
  typename LatticeType::DirectionType domainDirection;
  domainOrigin.Fill(0.0);
  domainSpacing.Fill(1.0);
  domainSize.Fill(4);
  meshSize.Fill(4);
  domainDirection.SetIdentity();
  for (unsigned int d = 0; d < Dim; ++d)
  {
    domainOrigin[d] = domain->GetOrigin()[d];
    domainSpacing[d] = domain->GetSpacing()[d];
    domainSize[d] = domain->GetBufferedRegion().GetSize()[d];
    meshSize[d] = 4;
    for (unsigned int j = 0; j < Dim; ++j)
    {
      domainDirection[d][j] = domain->GetDirection()[d][j];
    }
  }
  auto adaptor = AdaptorType::New();
  adaptor->SetSplineOrder(output->GetSplineOrder());
  adaptor->SetRequiredTransformDomainOrigin(domainOrigin);
  adaptor->SetRequiredTransformDomainSpacing(domainSpacing);
  adaptor->SetRequiredTransformDomainSize(domainSize);
  adaptor->SetRequiredTransformDomainDirection(domainDirection);
  adaptor->SetRequiredTransformDomainMeshSize(meshSize);

  constexpr VectorType zeroVector{};
  auto                 lattice = LatticeType::New();
  lattice->SetOrigin(adaptor->GetRequiredControlPointLatticeOrigin());
  lattice->SetSpacing(adaptor->GetRequiredControlPointLatticeSpacing());
  lattice->SetDirection(adaptor->GetRequiredControlPointLatticeDirection());
  lattice->SetRegions(adaptor->GetRequiredControlPointLatticeSize());
  lattice->Allocate();
  lattice->FillBuffer(zeroVector);

  output->SetTimeVaryingVelocityFieldControlPointLattice(lattice);
  output->SetVelocityFieldOrigin(domainOrigin);
  output->SetVelocityFieldSpacing(domainSpacing);
  output->SetVelocityFieldSize(domainSize);
  output->SetVelocityFieldDirection(domainDirection);
  output->IntegrateVelocityField();

  typename RegistrationType::TransformParametersAdaptorsContainerType adaptors;
  adaptors.push_back(adaptor.GetPointer());
  registration->SetTransformParametersAdaptorsPerLevel(adaptors);
  registration->SetInitialTransform(output);
  registration->InPlaceOn();
  registration->Update();
  return static_cast<double>(registration->GetMetric()->GetCurrentValue());
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
    std::cout << "module,operator,ms_float,ms_double,speedup,max_abs,rmse\n";
  }
  const FImg * src = input.GetPointer();

  double vecErr = 0.0;
  BenchTime(
    "ImageFilterBase",
    "VectorNeighborhoodOperatorImageFilter",
    [&]() { RunVector<float>(src); },
    [&]() { RunVector<double>(src); },
    runs,
    [&]() {
      if (vecErr == 0.0)
      {
        vecErr = VecMaxAbs(src);
      }
      return vecErr;
    });

  typename itk::Image<std::complex<float>, Dim>::Pointer  freqF;
  typename itk::Image<std::complex<double>, Dim>::Pointer freqD;
  double                                                  freqErr = 0.0;
  BenchTime(
    "ImageFrequency",
    "UnaryFrequencyDomainFilter",
    [&]() {
      if (!freqF)
      {
        freqF = ToFreq<float>(src, 64);
      }
      RunFreq<float>(freqF);
    },
    [&]() {
      if (!freqD)
      {
        freqD = ToFreq<double>(src, 64);
      }
      RunFreq<double>(freqD);
    },
    runs,
    [&]() {
      if (!freqF)
      {
        freqF = ToFreq<float>(src, 64);
      }
      if (!freqD)
      {
        freqD = ToFreq<double>(src, 64);
      }
      freqErr = FreqMaxAbs(freqF, freqD);
      return freqErr;
    });

  double mF = 0.0;
  double mD = 0.0;
  BenchTime(
    "RegistrationMethodsv4",
    "BSplineSyNImageRegistrationMethod",
    [&]() { mF = RunBSplineSyN<float>(src); },
    [&]() { mD = RunBSplineSyN<double>(src); },
    runs,
    [&]() { return std::abs(mF - mD); });

  mF = 0.0;
  mD = 0.0;
  BenchTime(
    "RegistrationMethodsv4",
    "TimeVaryingBSplineVelocityFieldImageRegistrationMethod",
    [&]() { mF = RunTimeVaryingBSpline<float>(src); },
    [&]() { mD = RunTimeVaryingBSpline<double>(src); },
    runs,
    [&]() { return std::abs(mF - mD); });
  return 0;
}
