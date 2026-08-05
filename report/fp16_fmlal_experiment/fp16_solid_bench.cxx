// FP16 solid 基准：按内核类型单次严格测试，再映射到 ITK 模块
// 方法：warmup + 交错 F32/F16 + 中位数 + min/max
// 用法: fp16_solid_bench [raw.f32] [w] [h] [paired_runs=30] [warmup=5] [threads=96]
#include "fp16_kernels.hxx"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

struct KernelSpec
{
  const char * key;
  const char * desc;
};

struct KernelResult
{
  std::string key;
  double      medF32{ 0 };
  double      medF16{ 0 };
  double      minF32{ 0 };
  double      maxF32{ 0 };
  double      minF16{ 0 };
  double      maxF16{ 0 };
  double      speedup{ 0 };
  double      maxAbs{ 0 };
  int         pairs{ 0 };
};

struct ModuleMap
{
  int         id;
  const char * name;
  const char * kernel;
  const char * note;
  bool        skip;
};

static int
ParseInt(int argc, char ** argv, int idx, int def)
{
  return argc > idx ? std::stoi(argv[idx]) : def;
}

static bool
LoadRawFloat(const std::string & path, int w, int h, std::vector<float> & img)
{
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;
  img.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
  in.read(reinterpret_cast<char *>(img.data()), static_cast<std::streamsize>(img.size() * sizeof(float)));
  return static_cast<bool>(in);
}

static void
FillSynthetic(std::vector<float> & img, int w, int h)
{
  img.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      img[static_cast<size_t>(y) * w + x] =
        200.f * std::sin(0.01f * x) * std::cos(0.013f * y) + 400.f + 0.3f * x;
    }
  }
}

static double
Median(std::vector<double> v)
{
  if (v.empty())
    return 0.0;
  std::sort(v.begin(), v.end());
  const size_t n = v.size();
  if (n % 2 == 1)
    return v[n / 2];
  return 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

static double
TimeOnce(const std::function<void()> & fn)
{
  const auto t0 = Clock::now();
  fn();
  return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

static double
TrimmedMedian(std::vector<double> v, double outlierFactor = 5.0)
{
  if (v.empty())
    return 0.0;
  const double rawMed = Median(v);
  const double hi = std::max(rawMed * outlierFactor, rawMed + 1.0);
  v.erase(std::remove_if(v.begin(), v.end(), [&](double x) { return x > hi; }), v.end());
  return v.empty() ? rawMed : Median(v);
}

static KernelResult
BenchInterleaved(const std::function<void()> & f32,
                 const std::function<void()> & f16,
                 int                           warmup,
                 int                           pairs)
{
  for (int i = 0; i < warmup; ++i)
  {
    f32();
    f16();
  }

  std::vector<double> tF32;
  std::vector<double> tF16;
  tF32.reserve(static_cast<size_t>(pairs));
  tF16.reserve(static_cast<size_t>(pairs));

  for (int i = 0; i < pairs; ++i)
  {
    tF32.push_back(TimeOnce(f32));
    tF16.push_back(TimeOnce(f16));
  }

  KernelResult r;
  r.pairs = pairs;
  r.medF32 = TrimmedMedian(tF32);
  r.medF16 = TrimmedMedian(tF16);
  r.minF32 = *std::min_element(tF32.begin(), tF32.end());
  r.maxF32 = *std::max_element(tF32.begin(), tF32.end());
  r.minF16 = *std::min_element(tF16.begin(), tF16.end());
  r.maxF16 = *std::max_element(tF16.begin(), tF16.end());
  r.speedup = r.medF32 / std::max(r.medF16, 1e-9);
  return r;
}

static const KernelResult *
FindKernel(const std::vector<KernelResult> & results, const char * key)
{
  for (const KernelResult & r : results)
  {
    if (r.key == key)
      return &r;
  }
  return nullptr;
}

static void
PrintKernelRow(const KernelResult & r)
{
  std::cout << std::fixed << std::setprecision(4);
  std::cout << r.key << "|median_f32=" << r.medF32 << "|median_f16=" << r.medF16 << "|speedup=" << r.speedup
            << "|f32[min,max]=[" << r.minF32 << ',' << r.maxF32 << "]|f16[min,max]=[" << r.minF16 << ',' << r.maxF16
            << "]|max_abs=" << std::setprecision(6) << r.maxAbs << std::setprecision(4) << "|pairs=" << r.pairs
            << '\n';
}

int
main(int argc, char ** argv)
{
  const std::string rawPath = argc > 1 ? argv[1] : "";
  const int         w = ParseInt(argc, argv, 2, 1024);
  const int         h = ParseInt(argc, argv, 3, 1024);
  const int         pairs = ParseInt(argc, argv, 4, 30);
  const int         warmup = ParseInt(argc, argv, 5, 5);
  const int         threads = ParseInt(argc, argv, 6, 96);

  omp_set_num_threads(threads);
  omp_set_dynamic(0);

  std::vector<float>     imgF;
  std::vector<float16_t> imgH;
  if (!rawPath.empty() && LoadRawFloat(rawPath, w, h, imgF))
  {
    std::cout << "loaded " << rawPath << '\n';
  }
  else
  {
    FillSynthetic(imgF, w, h);
    std::cout << "synthetic data\n";
  }
  FloatToHalfImage(imgF, imgH);

  std::vector<float16_t> imgH2 = imgH;
  for (size_t i = 0; i < imgH2.size(); i += 97)
  {
    imgH2[i] = vcvt_f16_f32(vdupq_n_f32(static_cast<float>(imgF[i]) + 3.f))[0];
  }

  std::vector<float> tmpF32, outF32, outF16;
  const int          radius = 15;
  const float        sigma = static_cast<float>(radius) / 3.f;
  const int          br = 4;

  std::cout << "=== fp16_solid_bench ===\n";
  std::cout << "method=warmup+" << warmup << " interleaved_pairs=" << pairs
            << " trimmed_median(outlier>5x)\n";
  std::cout << "size=" << w << 'x' << h << " threads=" << threads << " omp_max=" << omp_get_max_threads() << '\n';
  std::cout << "\n# --- kernel-level (each kernel tested ONCE) ---\n";

  std::vector<KernelResult> results;

  auto run = [&](const char * key, auto f32fn, auto f16fn) {
    KernelResult kr = BenchInterleaved(
      [&]() { f32fn(outF32); }, [&]() { f16fn(outF16); }, warmup, pairs);
    kr.key = key;
    kr.maxAbs = MaxAbsDiff(outF32, outF16);
    results.push_back(kr);
    PrintKernelRow(kr);
  };

  run("box_mean_r15",
      [&](std::vector<float> & o) { BoxMeanF32(imgF, o, w, h, radius); },
      [&](std::vector<float> & o) { BoxMeanF16(imgH, o, w, h, radius); });

  run("box_mean_r4",
      [&](std::vector<float> & o) { BoxMeanF32(imgF, o, w, h, br); },
      [&](std::vector<float> & o) { BoxMeanF16(imgH, o, w, h, br); });

  run("separable_gaussian_r15",
      [&](std::vector<float> & o) { SeparableGaussianF32(imgF, tmpF32, o, w, h, radius, sigma); },
      [&](std::vector<float> & o) { SeparableGaussianF16(imgH, tmpF32, o, w, h, radius, sigma); });

  run("bilateral_r4",
      [&](std::vector<float> & o) { BilateralDomainF32(imgF, o, w, h, br, 0.f); },
      [&](std::vector<float> & o) { BilateralDomainF16(imgH, o, w, h, br); });

  run("gradient_mag",
      [&](std::vector<float> & o) { GradientMagF32(imgF, o, w, h); },
      [&](std::vector<float> & o) { GradientMagF16(imgH, o, w, h); });

  run("median3",
      [&](std::vector<float> & o) { Median3F32(imgF, o, w, h); },
      [&](std::vector<float> & o) { Median3F16(imgH, o, w, h); });

  run("pointwise_scale",
      [&](std::vector<float> & o) { PointwiseScaleF32(imgF, o, 1.02f); },
      [&](std::vector<float> & o) { PointwiseScaleF16(imgH, o, 1.02f); });

  run("sum_scan",
      [&](std::vector<float> & o) { o.assign(1, SumF32(imgF)); },
      [&](std::vector<float> & o) { o.assign(1, SumF16(imgH)); });

  run("local_mse_r4",
      [&](std::vector<float> & o) { LocalMseF32(imgF, imgF, o, w, h, br); },
      [&](std::vector<float> & o) { LocalMseF16(imgH, imgH2, o, w, h, br); });

  // 分类
  std::cout << "\n# --- kernel verdict (|speedup-1| > 5% => meaningful) ---\n";
  double sumSp = 0;
  int    cntSp = 0;
  for (const KernelResult & r : results)
  {
    const double delta = std::fabs(r.speedup - 1.0);
    const char * verdict = delta >= 0.05 ? (r.speedup > 1.0 ? "F16_FASTER" : "F16_SLOWER") : "NOISE";
    std::cout << r.key << "|speedup=" << r.speedup << "|delta=" << std::setprecision(4) << delta << "|" << verdict
              << '\n';
    sumSp += r.speedup;
    ++cntSp;
  }
  std::cout << "# kernel_avg_speedup=" << (cntSp ? sumSp / cntSp : 0) << '\n';

  // 模块映射（不再重复跑内核）
  const ModuleMap modules[] = {
    { 1, "AnisotropicSmoothing", "separable_gaussian_r15", "CAD/GAD", false },
    { 2, "AntiAlias", "box_mean_r4", "", false },
    { 3, "BiasCorrection", "pointwise_scale", "", false },
    { 4, "BinaryMathematicalMorphology", "skip", "二值", true },
    { 5, "Colormap", "pointwise_scale", "", false },
    { 6, "Convolution", "separable_gaussian_r15", "", false },
    { 7, "CurvatureFlow", "separable_gaussian_r15", "", false },
    { 8, "Deconvolution", "separable_gaussian_r15", "", false },
    { 9, "Denoising", "separable_gaussian_r15", "", false },
    { 10, "DiffusionTensorImage", "gradient_mag", "", false },
    { 11, "DisplacementField", "pointwise_scale", "", false },
    { 12, "DistanceMap", "box_mean_r4", "", false },
    { 13, "FastMarching", "sum_scan", "", false },
    { 14, "FFT", "skip", "FFTW", true },
    { 15, "GPUAnisotropicSmoothing", "separable_gaussian_r15", "CPU equiv", false },
    { 16, "GPUImageFilterBase", "skip", "基类", true },
    { 17, "GPUSmoothing", "box_mean_r15", "CPU equiv", false },
    { 18, "GPUThresholding", "pointwise_scale", "CPU equiv", false },
    { 19, "ImageCompare", "sum_scan", "", false },
    { 20, "ImageCompose", "pointwise_scale", "", false },
    { 21, "ImageFeature", "bilateral_r4", "", false },
    { 22, "ImageFilterBase", "skip", "基类", true },
    { 23, "ImageFrequency", "skip", "FFT", true },
    { 24, "ImageFusion", "pointwise_scale", "", false },
    { 25, "ImageGradient", "gradient_mag", "", false },
    { 26, "ImageGrid", "box_mean_r4", "", false },
    { 27, "ImageIntensity", "pointwise_scale", "", false },
    { 28, "ImageLabel", "skip", "标签", true },
    { 29, "ImageNoise", "pointwise_scale", "", false },
    { 30, "ImageSources", "pointwise_scale", "", false },
    { 31, "ImageStatistics", "sum_scan", "", false },
    { 32, "LabelMap", "skip", "标签", true },
    { 33, "MathematicalMorphology", "median3", "", false },
    { 34, "Path", "gradient_mag", "", false },
    { 35, "QuadEdgeMeshFiltering", "skip", "网格", true },
    { 36, "Smoothing", "box_mean_r15", "BoxMean", false },
    { 37, "SpatialFunction", "skip", "基类", true },
    { 38, "Thresholding", "pointwise_scale", "", false },
  };

  std::cout << "\n# --- module map (inherits kernel median; NOT re-benchmarked) ---\n";
  std::cout << "# id|module|kernel|speedup|max_abs|verdict\n";
  double modSum = 0;
  int    modCnt = 0;
  for (const ModuleMap & m : modules)
  {
    if (m.skip)
    {
      std::cout << m.id << '|' << m.name << "|N/A|—|—|SKIP\n";
      continue;
    }
    const KernelResult * kr = FindKernel(results, m.kernel);
    if (!kr)
    {
      std::cout << m.id << '|' << m.name << "|missing|—|—|ERROR\n";
      continue;
    }
    const double delta = std::fabs(kr->speedup - 1.0);
    const char * verdict = delta >= 0.05 ? (kr->speedup > 1.0 ? "F16_FASTER" : "F16_SLOWER") : "NOISE";
    std::cout << m.id << '|' << m.name << '|' << m.kernel << '|' << std::setprecision(4) << kr->speedup << '|'
              << std::setprecision(6) << kr->maxAbs << '|' << verdict << '\n';
    modSum += kr->speedup;
    ++modCnt;
  }

  std::cout << "# module_avg_speedup=" << std::setprecision(4) << (modCnt ? modSum / modCnt : 0) << " (from "
            << modCnt << " mapped modules)\n";

  std::cout << "\n# --- Registration / Segmentation ---\n";
  for (const char * name :
       { "Metricsv4", "RegistrationMethodsv4", "LevelSets", "Watersheds", "ConnectedComponents" })
  {
    const char * k = (std::string(name) == "Metricsv4") ? "local_mse_r4" : "separable_gaussian_r15";
    const KernelResult * kr = FindKernel(results, k);
    const double         delta = kr ? std::fabs(kr->speedup - 1.0) : 0;
    const char *         verdict =
      !kr ? "ERROR" : (delta >= 0.05 ? (kr->speedup > 1.0 ? "F16_FASTER" : "F16_SLOWER") : "NOISE");
    std::cout << "R|" << name << '|' << k << '|' << (kr ? kr->speedup : 0) << '|' << (kr ? kr->maxAbs : 0) << '|'
              << verdict << '\n';
  }

  return 0;
}
