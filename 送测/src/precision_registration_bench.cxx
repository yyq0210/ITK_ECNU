// precision_registration_bench — Metricsv4: float 存储 + double 度量/变换 vs 全程 double 图像
// 用法: precision_registration_bench <fixed.png> <moving.png> [maxIter=50] [runs=3]
#include "itkCastImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegistrationMethodv4.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkMultiThreaderBase.h"
#include "itkPNGImageIOFactory.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"
#include "itkTranslationTransform.h"

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
}

constexpr unsigned int Dim = 2;

template <typename PixelType>
struct RegResult
{
  double                         ms{ 0.0 };
  double                         finalMetric{ 0.0 };
  itk::TranslationTransform<double, Dim>::ParametersType params;
};

template <typename PixelType>
static RegResult<PixelType>
RunRegistration(const std::string & fixedPath,
                const std::string & movingPath,
                unsigned int        maxIter)
{
  using ImageType = itk::Image<PixelType, Dim>;
  using TransformType = itk::TranslationTransform<double, Dim>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
  using RegistrationType = itk::ImageRegistrationMethodv4<ImageType, ImageType, TransformType>;

  using Reader = itk::ImageFileReader<ImageType>;
  auto fixedReader = Reader::New();
  auto movingReader = Reader::New();
  fixedReader->SetFileName(fixedPath);
  movingReader->SetFileName(movingPath);
  fixedReader->Update();
  movingReader->Update();

  auto metric = MetricType::New();
  auto optimizer = OptimizerType::New();
  auto registration = RegistrationType::New();

  using Interp = itk::LinearInterpolateImageFunction<ImageType, double>;
  auto fi = Interp::New();
  auto mi = Interp::New();
  metric->SetFixedInterpolator(fi);
  metric->SetMovingInterpolator(mi);

  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetFixedImage(fixedReader->GetOutput());
  registration->SetMovingImage(movingReader->GetOutput());

  auto movingInit = TransformType::New();
  TransformType::ParametersType p(movingInit->GetNumberOfParameters());
  p.Fill(0.0);
  movingInit->SetParameters(p);
  registration->SetMovingInitialTransform(movingInit);

  auto fixedInit = TransformType::New();
  fixedInit->SetIdentity();
  registration->SetFixedInitialTransform(fixedInit);

  optimizer->SetLearningRate(4.0);
  optimizer->SetMinimumStepLength(0.001);
  optimizer->SetRelaxationFactor(0.5);
  optimizer->SetNumberOfIterations(maxIter);

  const auto t0 = Clock::now();
  registration->Update();
  const auto t1 = Clock::now();

  RegResult<PixelType> r;
  r.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
  r.finalMetric = optimizer->GetValue();
  r.params = registration->GetTransform()->GetParameters();
  return r;
}

static RegResult<double>
RunDoubleViaCast(const std::string & fixedPath, const std::string & movingPath, unsigned int maxIter)
{
  using FImg = itk::Image<float, Dim>;
  using DImg = itk::Image<double, Dim>;
  using Cast = itk::CastImageFilter<FImg, DImg>;

  auto readCast = [&](const std::string & path) {
    using Reader = itk::ImageFileReader<FImg>;
    auto r = Reader::New();
    r->SetFileName(path);
    r->Update();
    auto c = Cast::New();
    c->SetInput(r->GetOutput());
    c->Update();
    DImg::Pointer out = c->GetOutput();
    out->DisconnectPipeline();
    return out;
  };

  const auto fixedPathTmp = fixedPath;
  const auto movingPathTmp = movingPath;

  // 写临时 double 到内存：直接用 reader float + cast 在 RunRegistration 外不可复用路径
  // 改为：读 float 文件路径仍有效，单独模板实例 RunRegistration<double> 需 double 文件；
  // 此处用 Cast 链写入临时逻辑 — 直接内联 double 配准并 cast 输入
  using ImageType = DImg;
  using TransformType = itk::TranslationTransform<double, Dim>;
  using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
  using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
  using RegistrationType = itk::ImageRegistrationMethodv4<ImageType, ImageType, TransformType>;

  auto fixedD = readCast(fixedPathTmp);
  auto movingD = readCast(movingPathTmp);

  auto metric = MetricType::New();
  auto optimizer = OptimizerType::New();
  auto registration = RegistrationType::New();
  using Interp = itk::LinearInterpolateImageFunction<ImageType, double>;
  metric->SetFixedInterpolator(Interp::New());
  metric->SetMovingInterpolator(Interp::New());
  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetFixedImage(fixedD);
  registration->SetMovingImage(movingD);

  auto movingInit = TransformType::New();
  TransformType::ParametersType p(movingInit->GetNumberOfParameters());
  p.Fill(0.0);
  movingInit->SetParameters(p);
  registration->SetMovingInitialTransform(movingInit);
  auto fixedInit = TransformType::New();
  fixedInit->SetIdentity();
  registration->SetFixedInitialTransform(fixedInit);

  optimizer->SetLearningRate(4.0);
  optimizer->SetMinimumStepLength(0.001);
  optimizer->SetRelaxationFactor(0.5);
  optimizer->SetNumberOfIterations(maxIter);

  const auto t0 = Clock::now();
  registration->Update();
  const auto t1 = Clock::now();

  RegResult<double> r;
  r.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
  r.finalMetric = optimizer->GetValue();
  r.params = registration->GetTransform()->GetParameters();
  return r;
}

int
main(int argc, char ** argv)
{
  if (argc < 3)
  {
    std::cerr << "usage: " << argv[0] << " <fixed.png> <moving.png> [maxIter=50] [runs=3]\n";
    return 1;
  }
  RegisterIO();
  const std::string fixedPath = argv[1];
  const std::string movingPath = argv[2];
  const unsigned int maxIter = argc >= 4 ? static_cast<unsigned int>(std::stoul(argv[3])) : 50;
  const int          runs = argc >= 5 ? std::stoi(argv[4]) : 3;

  std::cout << "=== precision_registration_bench ===\n";
  std::cout << "fixed=" << fixedPath << " moving=" << movingPath << " maxIter=" << maxIter
            << " runs=" << runs << " threads=" << itk::MultiThreaderBase::GetGlobalDefaultNumberOfThreads()
            << "\n\n";

  std::cout << "模式 A — Metricsv4 推荐: Image<float> + TranslationTransform<double> + "
               "Metric/Optimizer<double>\n";
  double msFloat = 0.0;
  RegResult<float> lastF;
  for (int i = 0; i < runs; ++i)
  {
    lastF = RunRegistration<float>(fixedPath, movingPath, maxIter);
    msFloat += lastF.ms;
  }
  msFloat /= runs;

  std::cout << "模式 B — 对照: Cast→Image<double> 存储 + 同样 double 度量/变换\n";
  double msDouble = 0.0;
  RegResult<double> lastD;
  for (int i = 0; i < runs; ++i)
  {
    lastD = RunDoubleViaCast(fixedPath, movingPath, maxIter);
    msDouble += lastD.ms;
  }
  msDouble /= runs;

  const double dTx = std::fabs(static_cast<double>(lastF.params[0]) - lastD.params[0]);
  const double dTy = std::fabs(static_cast<double>(lastF.params[1]) - lastD.params[1]);
  const double dMetric = std::fabs(lastF.finalMetric - lastD.finalMetric);

  std::cout << std::setprecision(8);
  std::cout << "\n--- 墙钟 ms (avg of " << runs << ") ---\n";
  std::cout << "float_storage:  " << msFloat << " ms\n";
  std::cout << "double_storage: " << msDouble << " ms  (含 cast 读入)\n";
  std::cout << "speedup (double/float): " << (msDouble / msFloat) << "x\n";
  std::cout << "\n--- 收敛结果 (最后一次) ---\n";
  std::cout << "float  final_metric=" << lastF.finalMetric << " tx=" << lastF.params[0]
            << " ty=" << lastF.params[1] << '\n';
  std::cout << "double final_metric=" << lastD.finalMetric << " tx=" << lastD.params[0]
            << " ty=" << lastD.params[1] << '\n';
  std::cout << "delta_metric=" << dMetric << " delta_tx=" << dTx << " delta_ty=" << dTy << '\n';
  std::cout << "\n说明: ImageRegistration1 即模式 A；度量/插值/优化器均为 double，仅像素存储为 float。\n";

  return 0;
}
