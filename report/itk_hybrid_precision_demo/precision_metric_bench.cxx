// 造数据测配准度量 / 点云距离：固定图+平移移动图，或从图像抽样点集。
// precision_metric_bench --list
// precision_metric_bench <image.png> [runs=3] [OperatorName]
#include "itkANTSNeighborhoodCorrelationImageToImageMetricv4.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBSplineScatteredDataPointSetToImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCorrelationCoefficientHistogramImageToImageMetric.h"
#include "itkCorrelationImageToImageMetricv4.h"
#include "itkDemonsImageToImageMetricv4.h"
#include "itkDisplacementFieldTransform.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkEuclideanDistancePointSetToPointSetMetricv4.h"
#include "itkExpectationBasedPointSetToPointSetMetricv4.h"
#include "itkGradientDifferenceImageToImageMetric.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkJensenHavrdaCharvatTsallisPointSetToPointSetMetricv4.h"
#include "itkJointHistogramMutualInformationImageToImageMetricv4.h"
#include "itkKappaStatisticImageToImageMetric.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "itkKullbackLeiblerCompareHistogramImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMatchCardinalityImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetricv4.h"
#include "itkMeanReciprocalSquareDifferenceImageToImageMetric.h"
#include "itkMeanReciprocalSquareDifferencePointSetToImageMetric.h"
#include "itkMeanSquaresHistogramImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkMeanSquaresPointSetToImageMetric.h"
#include "itkMetaImageIOFactory.h"
#include "itkMultiThreaderBase.h"
#include "itkMutualInformationHistogramImageToImageMetric.h"
#include "itkMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkNormalizedCorrelationPointSetToImageMetric.h"
#include "itkNormalizedMutualInformationHistogramImageToImageMetric.h"
#include "itkObjectToObjectMultiMetricv4.h"
#include "itkPNGImageIOFactory.h"
#include "itkPointSet.h"
#include "itkPointSetToImageRegistrationMethod.h"
#include "itkPointSetToPointSetRegistrationMethod.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkResampleImageFilter.h"
#include "itkTranslationTransform.h"
#include "itkVector.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;
using PSF = itk::PointSet<float, Dim>;
using PSD = itk::PointSet<double, Dim>;
using Trans = itk::TranslationTransform<double, Dim>;

static std::string g_only;
static bool        g_list = false;
static constexpr int kStride = 8;
static constexpr int kStrideKernel = 32;

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
Shift(const TImage * in, double dx, double dy)
{
  using Res = itk::ResampleImageFilter<TImage, TImage>;
  auto t = Trans::New();
  typename Trans::OutputVectorType off;
  off[0] = dx;
  off[1] = dy;
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

template <typename TImage>
static typename TImage::Pointer
Bin(const TImage * in)
{
  using F = itk::BinaryThresholdImageFilter<TImage, TImage>;
  auto f = F::New();
  f->SetInput(in);
  f->SetLowerThreshold(40);
  f->SetUpperThreshold(255);
  f->SetInsideValue(1);
  f->SetOutsideValue(0);
  f->Update();
  return Hold(f->GetOutput());
}

template <typename TImage, typename TPointSet>
static typename TPointSet::Pointer
SamplePoints(const TImage * img, int stride, bool withData)
{
  auto ps = TPointSet::New();
  using PointType = typename TPointSet::PointType;
  unsigned int id = 0;
  itk::ImageRegionConstIteratorWithIndex<TImage> it(img, img->GetBufferedRegion());
  for (; !it.IsAtEnd(); ++it)
  {
    const auto idx = it.GetIndex();
    if ((idx[0] % stride) != 0 || (idx[1] % stride) != 0)
    {
      continue;
    }
    PointType pt;
    img->TransformIndexToPhysicalPoint(idx, pt);
    ps->SetPoint(id, pt);
    if (withData)
    {
      ps->SetPointData(id, static_cast<typename TPointSet::PixelType>(it.Get()));
    }
    ++id;
  }
  return ps;
}

template <typename TPointSet>
static typename TPointSet::Pointer
OffsetPoints(const TPointSet * in, double dx, double dy)
{
  auto               out = TPointSet::New();
  const unsigned int n = static_cast<unsigned int>(in->GetNumberOfPoints());
  for (unsigned int i = 0; i < n; ++i)
  {
    auto p = in->GetPoint(i);
    p[0] += static_cast<double>(dx);
    p[1] += static_cast<double>(dy);
    out->SetPoint(i, p);
    typename TPointSet::PixelType d{};
    if (in->GetPointData(i, &d))
    {
      out->SetPointData(i, d);
    }
  }
  return out;
}

template <typename TMetric, typename TImage>
static void
RunV3(TMetric * metric, TImage * fixed, TImage * moving)
{
  using Interp = itk::LinearInterpolateImageFunction<TImage, double>;
  auto tr = Trans::New();
  tr->SetIdentity();
  auto ip = Interp::New();
  metric->SetFixedImage(fixed);
  metric->SetMovingImage(moving);
  metric->SetTransform(tr);
  metric->SetInterpolator(ip);
  metric->SetFixedImageRegion(fixed->GetBufferedRegion());
  metric->Initialize();
  (void)metric->GetValue(tr->GetParameters());
}

template <typename TMetric, typename TImage>
static void
RunV4(TMetric * metric, TImage * fixed, TImage * moving)
{
  auto tr = Trans::New();
  tr->SetIdentity();
  metric->SetFixedImage(fixed);
  metric->SetMovingImage(moving);
  metric->SetMovingTransform(tr);
  metric->Initialize();
  (void)metric->GetValue();
}

template <typename TMetric>
static void
HistSize(TMetric * metric)
{
  typename TMetric::HistogramSizeType hs;
  hs.SetSize(2);
  hs.Fill(50);
  metric->SetHistogramSize(hs);
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

  FImg::Pointer fixedF, movingF, binF, binMovingF;
  DImg::Pointer fixedD, movingD, binD, binMovingD;
  PSF::Pointer  ptsF, ptsMovingF, ptsSparseF, ptsSparseMovingF;
  PSD::Pointer  ptsD, ptsMovingD, ptsSparseD, ptsSparseMovingD;
  PSD::Pointer  ptsFromF, ptsMovingFromF, ptsSparseFromF, ptsSparseMovingFromF;

  if (!g_list)
  {
    using Reader = itk::ImageFileReader<FImg>;
    auto reader = Reader::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    fixedF = Hold(reader->GetOutput());
    movingF = Shift(fixedF.GetPointer(), 5.0, 3.0);
    using Cast = itk::CastImageFilter<FImg, DImg>;
    auto c1 = Cast::New();
    c1->SetInput(fixedF);
    c1->Update();
    fixedD = Hold(c1->GetOutput());
    auto c2 = Cast::New();
    c2->SetInput(movingF);
    c2->Update();
    movingD = Hold(c2->GetOutput());
    binF = Bin(fixedF.GetPointer());
    binMovingF = Bin(movingF.GetPointer());
    binD = Bin(fixedD.GetPointer());
    binMovingD = Bin(movingD.GetPointer());
    ptsF = SamplePoints<FImg, PSF>(fixedF, kStride, true);
    ptsD = SamplePoints<DImg, PSD>(fixedD, kStride, true);
    ptsMovingF = OffsetPoints(ptsF.GetPointer(), 5.0, 3.0);
    ptsMovingD = OffsetPoints(ptsD.GetPointer(), 5.0, 3.0);
    ptsSparseF = SamplePoints<FImg, PSF>(fixedF, kStrideKernel, true);
    ptsSparseD = SamplePoints<DImg, PSD>(fixedD, kStrideKernel, true);
    ptsSparseMovingF = OffsetPoints(ptsSparseF.GetPointer(), 5.0, 3.0);
    ptsSparseMovingD = OffsetPoints(ptsSparseD.GetPointer(), 5.0, 3.0);
    ptsFromF = SamplePoints<FImg, PSD>(fixedF, kStride, true);
    ptsMovingFromF = OffsetPoints(ptsFromF.GetPointer(), 5.0, 3.0);
    ptsSparseFromF = SamplePoints<FImg, PSD>(fixedF, kStrideKernel, true);
    ptsSparseMovingFromF = OffsetPoints(ptsSparseFromF.GetPointer(), 5.0, 3.0);
    const auto sz = fixedF->GetLargestPossibleRegion().GetSize();
    std::cerr << "metric " << sz[0] << 'x' << sz[1] << " points=" << ptsF->GetNumberOfPoints()
              << " sparse=" << ptsSparseF->GetNumberOfPoints()
              << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads() << std::endl;
  }

  BenchTime(
    "Common",
    "MeanSquaresImageToImageMetric",
    [&]() {
      auto m = itk::MeanSquaresImageToImageMetric<FImg, FImg>::New();
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MeanSquaresImageToImageMetric<DImg, DImg>::New();
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "NormalizedCorrelationImageToImageMetric",
    [&]() {
      auto m = itk::NormalizedCorrelationImageToImageMetric<FImg, FImg>::New();
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::NormalizedCorrelationImageToImageMetric<DImg, DImg>::New();
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "MeanReciprocalSquareDifferenceImageToImageMetric",
    [&]() {
      auto m = itk::MeanReciprocalSquareDifferenceImageToImageMetric<FImg, FImg>::New();
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MeanReciprocalSquareDifferenceImageToImageMetric<DImg, DImg>::New();
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "GradientDifferenceImageToImageMetric",
    [&]() {
      auto m = itk::GradientDifferenceImageToImageMetric<FImg, FImg>::New();
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::GradientDifferenceImageToImageMetric<DImg, DImg>::New();
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "MattesMutualInformationImageToImageMetric",
    [&]() {
      auto m = itk::MattesMutualInformationImageToImageMetric<FImg, FImg>::New();
      m->SetNumberOfHistogramBins(50);
      m->SetNumberOfSpatialSamples(25000);
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MattesMutualInformationImageToImageMetric<DImg, DImg>::New();
      m->SetNumberOfHistogramBins(50);
      m->SetNumberOfSpatialSamples(25000);
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "MutualInformationImageToImageMetric",
    [&]() {
      auto m = itk::MutualInformationImageToImageMetric<FImg, FImg>::New();
      m->SetFixedImageStandardDeviation(20.0);
      m->SetMovingImageStandardDeviation(20.0);
      m->SetNumberOfSpatialSamples(20000);
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MutualInformationImageToImageMetric<DImg, DImg>::New();
      m->SetFixedImageStandardDeviation(20.0);
      m->SetMovingImageStandardDeviation(20.0);
      m->SetNumberOfSpatialSamples(20000);
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "MeanSquaresHistogramImageToImageMetric",
    [&]() {
      auto m = itk::MeanSquaresHistogramImageToImageMetric<FImg, FImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MeanSquaresHistogramImageToImageMetric<DImg, DImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "CorrelationCoefficientHistogramImageToImageMetric",
    [&]() {
      auto m = itk::CorrelationCoefficientHistogramImageToImageMetric<FImg, FImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::CorrelationCoefficientHistogramImageToImageMetric<DImg, DImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "MutualInformationHistogramImageToImageMetric",
    [&]() {
      auto m = itk::MutualInformationHistogramImageToImageMetric<FImg, FImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MutualInformationHistogramImageToImageMetric<DImg, DImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "NormalizedMutualInformationHistogramImageToImageMetric",
    [&]() {
      auto m = itk::NormalizedMutualInformationHistogramImageToImageMetric<FImg, FImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::NormalizedMutualInformationHistogramImageToImageMetric<DImg, DImg>::New();
      HistSize(m.GetPointer());
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "KullbackLeiblerCompareHistogramImageToImageMetric",
    [&]() {
      using M = itk::KullbackLeiblerCompareHistogramImageToImageMetric<FImg, FImg>;
      using Interp = itk::LinearInterpolateImageFunction<FImg, double>;
      auto m = M::New();
      HistSize(m.GetPointer());
      auto trT = Trans::New();
      trT->SetIdentity();
      auto ipT = Interp::New();
      m->SetTrainingFixedImage(fixedF);
      m->SetTrainingMovingImage(movingF);
      m->SetTrainingTransform(trT);
      m->SetTrainingInterpolator(ipT);
      m->SetTrainingFixedImageRegion(fixedF->GetBufferedRegion());
      RunV3(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      using M = itk::KullbackLeiblerCompareHistogramImageToImageMetric<DImg, DImg>;
      using Interp = itk::LinearInterpolateImageFunction<DImg, double>;
      auto m = M::New();
      HistSize(m.GetPointer());
      auto trT = Trans::New();
      trT->SetIdentity();
      auto ipT = Interp::New();
      m->SetTrainingFixedImage(fixedD);
      m->SetTrainingMovingImage(movingD);
      m->SetTrainingTransform(trT);
      m->SetTrainingInterpolator(ipT);
      m->SetTrainingFixedImageRegion(fixedD->GetBufferedRegion());
      RunV3(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "KappaStatisticImageToImageMetric",
    [&]() {
      auto m = itk::KappaStatisticImageToImageMetric<FImg, FImg>::New();
      RunV3(m.GetPointer(), binF.GetPointer(), binMovingF.GetPointer());
    },
    [&]() {
      auto m = itk::KappaStatisticImageToImageMetric<DImg, DImg>::New();
      RunV3(m.GetPointer(), binD.GetPointer(), binMovingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "MatchCardinalityImageToImageMetric",
    [&]() {
      auto m = itk::MatchCardinalityImageToImageMetric<FImg, FImg>::New();
      RunV3(m.GetPointer(), binF.GetPointer(), binMovingF.GetPointer());
    },
    [&]() {
      auto m = itk::MatchCardinalityImageToImageMetric<DImg, DImg>::New();
      RunV3(m.GetPointer(), binD.GetPointer(), binMovingD.GetPointer());
    },
    runs);

  BenchTime(
    "Common",
    "MeanSquaresPointSetToImageMetric",
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<FImg, double>;
      auto m = itk::MeanSquaresPointSetToImageMetric<PSF, FImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsF);
      m->SetMovingImage(movingF);
      m->SetTransform(tr);
      m->SetInterpolator(Interp::New());
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<DImg, double>;
      auto m = itk::MeanSquaresPointSetToImageMetric<PSD, DImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsD);
      m->SetMovingImage(movingD);
      m->SetTransform(tr);
      m->SetInterpolator(Interp::New());
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    runs);

  BenchTime(
    "Common",
    "MeanReciprocalSquareDifferencePointSetToImageMetric",
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<FImg, double>;
      auto m = itk::MeanReciprocalSquareDifferencePointSetToImageMetric<PSF, FImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsF);
      m->SetMovingImage(movingF);
      m->SetTransform(tr);
      m->SetInterpolator(Interp::New());
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<DImg, double>;
      auto m = itk::MeanReciprocalSquareDifferencePointSetToImageMetric<PSD, DImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsD);
      m->SetMovingImage(movingD);
      m->SetTransform(tr);
      m->SetInterpolator(Interp::New());
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    runs);

  BenchTime(
    "Common",
    "NormalizedCorrelationPointSetToImageMetric",
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<FImg, double>;
      auto m = itk::NormalizedCorrelationPointSetToImageMetric<PSF, FImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsF);
      m->SetMovingImage(movingF);
      m->SetTransform(tr);
      m->SetInterpolator(Interp::New());
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<DImg, double>;
      auto m = itk::NormalizedCorrelationPointSetToImageMetric<PSD, DImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsD);
      m->SetMovingImage(movingD);
      m->SetTransform(tr);
      m->SetInterpolator(Interp::New());
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    runs);

  BenchTime(
    "Common",
    "EuclideanDistancePointMetric",
    [&]() {
      auto m = itk::EuclideanDistancePointMetric<PSF, PSF>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsF);
      m->SetMovingPointSet(ptsMovingF);
      m->SetTransform(tr);
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    [&]() {
      auto m = itk::EuclideanDistancePointMetric<PSD, PSD>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m->SetFixedPointSet(ptsD);
      m->SetMovingPointSet(ptsMovingD);
      m->SetTransform(tr);
      m->Initialize();
      (void)m->GetValue(tr->GetParameters());
    },
    runs);

  BenchTime(
    "Common",
    "PointSetToImageRegistrationMethod",
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<FImg, double>;
      using Metric = itk::MeanSquaresPointSetToImageMetric<PSF, FImg>;
      using Method = itk::PointSetToImageRegistrationMethod<PSF, FImg>;
      auto metric = Metric::New();
      auto opt = itk::RegularStepGradientDescentOptimizer::New();
      auto method = Method::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      method->SetMetric(metric);
      method->SetOptimizer(opt);
      method->SetTransform(tr);
      method->SetInterpolator(Interp::New());
      method->SetFixedPointSet(ptsSparseF);
      method->SetMovingImage(movingF);
      method->SetInitialTransformParameters(tr->GetParameters());
      opt->SetMaximumStepLength(2.0);
      opt->SetMinimumStepLength(0.1);
      opt->SetNumberOfIterations(8);
      method->Update();
    },
    [&]() {
      using Interp = itk::LinearInterpolateImageFunction<DImg, double>;
      using Metric = itk::MeanSquaresPointSetToImageMetric<PSD, DImg>;
      using Method = itk::PointSetToImageRegistrationMethod<PSD, DImg>;
      auto metric = Metric::New();
      auto opt = itk::RegularStepGradientDescentOptimizer::New();
      auto method = Method::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      method->SetMetric(metric);
      method->SetOptimizer(opt);
      method->SetTransform(tr);
      method->SetInterpolator(Interp::New());
      method->SetFixedPointSet(ptsSparseD);
      method->SetMovingImage(movingD);
      method->SetInitialTransformParameters(tr->GetParameters());
      opt->SetMaximumStepLength(2.0);
      opt->SetMinimumStepLength(0.1);
      opt->SetNumberOfIterations(8);
      method->Update();
    },
    runs);

  BenchTime(
    "Common",
    "PointSetToPointSetRegistrationMethod",
    [&]() {
      using Metric = itk::EuclideanDistancePointMetric<PSF, PSF>;
      using Method = itk::PointSetToPointSetRegistrationMethod<PSF, PSF>;
      auto metric = Metric::New();
      auto opt = itk::LevenbergMarquardtOptimizer::New();
      auto method = Method::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      method->SetMetric(metric);
      method->SetOptimizer(opt);
      method->SetTransform(tr);
      method->SetFixedPointSet(ptsSparseF);
      method->SetMovingPointSet(ptsSparseMovingF);
      method->SetInitialTransformParameters(tr->GetParameters());
      opt->SetNumberOfIterations(8);
      opt->SetUseCostFunctionGradient(false);
      method->Update();
    },
    [&]() {
      using Metric = itk::EuclideanDistancePointMetric<PSD, PSD>;
      using Method = itk::PointSetToPointSetRegistrationMethod<PSD, PSD>;
      auto metric = Metric::New();
      auto opt = itk::LevenbergMarquardtOptimizer::New();
      auto method = Method::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      method->SetMetric(metric);
      method->SetOptimizer(opt);
      method->SetTransform(tr);
      method->SetFixedPointSet(ptsSparseD);
      method->SetMovingPointSet(ptsSparseMovingD);
      method->SetInitialTransformParameters(tr->GetParameters());
      opt->SetNumberOfIterations(8);
      opt->SetUseCostFunctionGradient(false);
      method->Update();
    },
    runs);

  BenchTime(
    "Metricsv4",
    "MeanSquaresImageToImageMetricv4",
    [&]() {
      auto m = itk::MeanSquaresImageToImageMetricv4<FImg, FImg>::New();
      RunV4(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MeanSquaresImageToImageMetricv4<DImg, DImg>::New();
      RunV4(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Metricsv4",
    "CorrelationImageToImageMetricv4",
    [&]() {
      auto m = itk::CorrelationImageToImageMetricv4<FImg, FImg>::New();
      RunV4(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::CorrelationImageToImageMetricv4<DImg, DImg>::New();
      RunV4(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Metricsv4",
    "DemonsImageToImageMetricv4",
    [&]() {
      using Vec = itk::Vector<double, 2>;
      using Field = itk::Image<Vec, 2>;
      using Disp = itk::DisplacementFieldTransform<double, 2>;
      auto field = Field::New();
      field->CopyInformation(fixedF);
      field->SetRegions(fixedF->GetLargestPossibleRegion());
      field->Allocate();
      Vec z;
      z.Fill(0);
      field->FillBuffer(z);
      auto tr = Disp::New();
      tr->SetDisplacementField(field);
      auto m = itk::DemonsImageToImageMetricv4<FImg, FImg>::New();
      m->SetFixedImage(fixedF);
      m->SetMovingImage(movingF);
      m->SetMovingTransform(tr);
      m->Initialize();
      (void)m->GetValue();
    },
    [&]() {
      using Vec = itk::Vector<double, 2>;
      using Field = itk::Image<Vec, 2>;
      using Disp = itk::DisplacementFieldTransform<double, 2>;
      auto field = Field::New();
      field->CopyInformation(fixedD);
      field->SetRegions(fixedD->GetLargestPossibleRegion());
      field->Allocate();
      Vec z;
      z.Fill(0);
      field->FillBuffer(z);
      auto tr = Disp::New();
      tr->SetDisplacementField(field);
      auto m = itk::DemonsImageToImageMetricv4<DImg, DImg>::New();
      m->SetFixedImage(fixedD);
      m->SetMovingImage(movingD);
      m->SetMovingTransform(tr);
      m->Initialize();
      (void)m->GetValue();
    },
    runs);

  BenchTime(
    "Metricsv4",
    "ANTSNeighborhoodCorrelationImageToImageMetricv4",
    [&]() {
      auto m = itk::ANTSNeighborhoodCorrelationImageToImageMetricv4<FImg, FImg>::New();
      typename itk::ANTSNeighborhoodCorrelationImageToImageMetricv4<FImg, FImg>::RadiusType radius;
      radius.Fill(2);
      m->SetRadius(radius);
      RunV4(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::ANTSNeighborhoodCorrelationImageToImageMetricv4<DImg, DImg>::New();
      typename itk::ANTSNeighborhoodCorrelationImageToImageMetricv4<DImg, DImg>::RadiusType radius;
      radius.Fill(2);
      m->SetRadius(radius);
      RunV4(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Metricsv4",
    "JointHistogramMutualInformationImageToImageMetricv4",
    [&]() {
      auto m = itk::JointHistogramMutualInformationImageToImageMetricv4<FImg, FImg>::New();
      m->SetNumberOfHistogramBins(50);
      RunV4(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::JointHistogramMutualInformationImageToImageMetricv4<DImg, DImg>::New();
      m->SetNumberOfHistogramBins(50);
      RunV4(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Metricsv4",
    "MattesMutualInformationImageToImageMetricv4",
    [&]() {
      auto m = itk::MattesMutualInformationImageToImageMetricv4<FImg, FImg>::New();
      m->SetNumberOfHistogramBins(50);
      RunV4(m.GetPointer(), fixedF.GetPointer(), movingF.GetPointer());
    },
    [&]() {
      auto m = itk::MattesMutualInformationImageToImageMetricv4<DImg, DImg>::New();
      m->SetNumberOfHistogramBins(50);
      RunV4(m.GetPointer(), fixedD.GetPointer(), movingD.GetPointer());
    },
    runs);

  BenchTime(
    "Metricsv4",
    "ObjectToObjectMultiMetricv4",
    [&]() {
      using Multi = itk::ObjectToObjectMultiMetricv4<Dim, Dim, FImg>;
      auto multi = Multi::New();
      auto m1 = itk::MeanSquaresImageToImageMetricv4<FImg, FImg>::New();
      auto m2 = itk::CorrelationImageToImageMetricv4<FImg, FImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m1->SetFixedImage(fixedF);
      m1->SetMovingImage(movingF);
      m1->SetMovingTransform(tr);
      m2->SetFixedImage(fixedF);
      m2->SetMovingImage(movingF);
      m2->SetMovingTransform(tr);
      multi->AddMetric(m1);
      multi->AddMetric(m2);
      multi->Initialize();
      (void)multi->GetValue();
    },
    [&]() {
      using Multi = itk::ObjectToObjectMultiMetricv4<Dim, Dim, DImg>;
      auto multi = Multi::New();
      auto m1 = itk::MeanSquaresImageToImageMetricv4<DImg, DImg>::New();
      auto m2 = itk::CorrelationImageToImageMetricv4<DImg, DImg>::New();
      auto tr = Trans::New();
      tr->SetIdentity();
      m1->SetFixedImage(fixedD);
      m1->SetMovingImage(movingD);
      m1->SetMovingTransform(tr);
      m2->SetFixedImage(fixedD);
      m2->SetMovingImage(movingD);
      m2->SetMovingTransform(tr);
      multi->AddMetric(m1);
      multi->AddMetric(m2);
      multi->Initialize();
      (void)multi->GetValue();
    },
    runs);

  BenchTime(
    "Metricsv4",
    "EuclideanDistancePointSetToPointSetMetricv4",
    [&]() {
      auto m = itk::EuclideanDistancePointSetToPointSetMetricv4<PSD>::New();
      m->SetFixedPointSet(ptsFromF);
      m->SetMovingPointSet(ptsMovingFromF);
      m->Initialize();
      (void)m->GetValue();
    },
    [&]() {
      auto m = itk::EuclideanDistancePointSetToPointSetMetricv4<PSD>::New();
      m->SetFixedPointSet(ptsD);
      m->SetMovingPointSet(ptsMovingD);
      m->Initialize();
      (void)m->GetValue();
    },
    runs);

  BenchTime(
    "Metricsv4",
    "ExpectationBasedPointSetToPointSetMetricv4",
    [&]() {
      auto m = itk::ExpectationBasedPointSetToPointSetMetricv4<PSD>::New();
      m->SetFixedPointSet(ptsSparseFromF);
      m->SetMovingPointSet(ptsSparseMovingFromF);
      m->SetPointSetSigma(2.0);
      m->Initialize();
      (void)m->GetValue();
    },
    [&]() {
      auto m = itk::ExpectationBasedPointSetToPointSetMetricv4<PSD>::New();
      m->SetFixedPointSet(ptsSparseD);
      m->SetMovingPointSet(ptsSparseMovingD);
      m->SetPointSetSigma(2.0);
      m->Initialize();
      (void)m->GetValue();
    },
    runs);

  BenchTime(
    "Metricsv4",
    "JensenHavrdaCharvatTsallisPointSetToPointSetMetricv4",
    [&]() {
      auto m = itk::JensenHavrdaCharvatTsallisPointSetToPointSetMetricv4<PSD>::New();
      m->SetFixedPointSet(ptsSparseFromF);
      m->SetMovingPointSet(ptsSparseMovingFromF);
      m->SetPointSetSigma(5.0);
      m->SetKernelSigma(4.0);
      m->SetAlpha(1.0);
      m->SetEvaluationKNeighborhood(16);
      m->SetCovarianceKNeighborhood(4);
      m->Initialize();
      (void)m->GetValue();
    },
    [&]() {
      auto m = itk::JensenHavrdaCharvatTsallisPointSetToPointSetMetricv4<PSD>::New();
      m->SetFixedPointSet(ptsSparseD);
      m->SetMovingPointSet(ptsSparseMovingD);
      m->SetPointSetSigma(5.0);
      m->SetKernelSigma(4.0);
      m->SetAlpha(1.0);
      m->SetEvaluationKNeighborhood(16);
      m->SetCovarianceKNeighborhood(4);
      m->Initialize();
      (void)m->GetValue();
    },
    runs);

  BenchTime(
    "ImageGrid",
    "BSplineScatteredDataPointSetToImageFilter",
    [&]() {
      using Vec = itk::Vector<float, 1>;
      using PS = itk::PointSet<Vec, 2>;
      using Out = itk::Image<Vec, 2>;
      using F = itk::BSplineScatteredDataPointSetToImageFilter<PS, Out>;
      auto ps = PS::New();
      unsigned int id = 0;
      itk::ImageRegionConstIteratorWithIndex<FImg> it(fixedF, fixedF->GetBufferedRegion());
      for (; !it.IsAtEnd(); ++it)
      {
        const auto idx = it.GetIndex();
        if ((idx[0] % 16) != 0 || (idx[1] % 16) != 0)
        {
          continue;
        }
        PS::PointType pt;
        fixedF->TransformIndexToPhysicalPoint(idx, pt);
        Vec v;
        v[0] = it.Get();
        ps->SetPoint(id, pt);
        ps->SetPointData(id, v);
        ++id;
      }
      auto f = F::New();
      f->SetInput(ps);
      f->SetSplineOrder(2);
      typename F::ArrayType nc;
      nc.Fill(6);
      f->SetNumberOfControlPoints(nc);
      f->SetSize(fixedF->GetLargestPossibleRegion().GetSize());
      f->SetSpacing(fixedF->GetSpacing());
      f->SetOrigin(fixedF->GetOrigin());
      f->SetDirection(fixedF->GetDirection());
      f->Update();
    },
    [&]() {
      using Vec = itk::Vector<double, 1>;
      using PS = itk::PointSet<Vec, 2>;
      using Out = itk::Image<Vec, 2>;
      using F = itk::BSplineScatteredDataPointSetToImageFilter<PS, Out>;
      auto ps = PS::New();
      unsigned int id = 0;
      itk::ImageRegionConstIteratorWithIndex<DImg> it(fixedD, fixedD->GetBufferedRegion());
      for (; !it.IsAtEnd(); ++it)
      {
        const auto idx = it.GetIndex();
        if ((idx[0] % 16) != 0 || (idx[1] % 16) != 0)
        {
          continue;
        }
        PS::PointType pt;
        fixedD->TransformIndexToPhysicalPoint(idx, pt);
        Vec v;
        v[0] = it.Get();
        ps->SetPoint(id, pt);
        ps->SetPointData(id, v);
        ++id;
      }
      auto f = F::New();
      f->SetInput(ps);
      f->SetSplineOrder(2);
      typename F::ArrayType nc;
      nc.Fill(6);
      f->SetNumberOfControlPoints(nc);
      f->SetSize(fixedD->GetLargestPossibleRegion().GetSize());
      f->SetSpacing(fixedD->GetSpacing());
      f->SetOrigin(fixedD->GetOrigin());
      f->SetDirection(fixedD->GetDirection());
      f->Update();
    },
    runs);

  return 0;
}
