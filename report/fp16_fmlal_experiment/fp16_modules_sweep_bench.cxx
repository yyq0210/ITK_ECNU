// 全模块 FP16 存储 + 拓宽（+fp16）性能 sweep
// 对比 F32 存储 vs F16 存储（读入 static_cast/fcvt 拓宽 + FP32 计算）
// 用法: fp16_modules_sweep_bench [raw_float_file] [w] [h] [runs] [threads]
//   无文件时用合成数据；文件为 little-endian float32 row-major
#include "fp16_kernels.hxx"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

struct ModuleCase
{
  int         id;
  const char * name;
  const char * kernel;
  const char * note;
  bool        skip;
};

struct BenchRow
{
  int    id;
  std::string name;
  std::string kernel;
  std::string note;
  double msF32{ 0 };
  double msF16{ 0 };
  double speedup{ 0 };
  double maxAbs{ 0 };
  std::string status;
};

static int
ParseInt(int argc, char ** argv, int idx, int def)
{
  return argc > idx ? std::stoi(argv[idx]) : def;
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

template <typename Fn>
static double
BenchMs(Fn fn, int runs)
{
  fn();
  double total = 0.0;
  for (int r = 0; r < runs; ++r)
  {
    const auto t0 = Clock::now();
    fn();
    total += std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  }
  return total / runs;
}

static void
RunPair(BenchRow &                 row,
        int                        runs,
        const std::function<void(std::vector<float> &)> & f32,
        const std::function<void(std::vector<float> &)> & f16,
        const std::vector<float> & ref)
{
  std::vector<float> outF32, outF16;
  row.msF32 = BenchMs([&]() { f32(outF32); }, runs);
  row.msF16 = BenchMs([&]() { f16(outF16); }, runs);
  row.speedup = row.msF32 / std::max(row.msF16, 1e-9);
  row.maxAbs = MaxAbsDiff(outF32, outF16);
  row.status = "OK";
  (void)ref;
}

int
main(int argc, char ** argv)
{
  const std::string rawPath = argc > 1 ? argv[1] : "";
  const int         w = ParseInt(argc, argv, 2, 1024);
  const int         h = ParseInt(argc, argv, 3, 1024);
  const int         runs = ParseInt(argc, argv, 4, 3);
  const int         threads = ParseInt(argc, argv, 5, 96);

  omp_set_num_threads(threads);

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

  std::vector<float> tmpF32, tmpF32b, out;
  const int          radius = 15;
  const float        sigma = static_cast<float>(radius) / 3.f;

  // 38 Filtering 模块映射
  const ModuleCase modules[] = {
    { 1, "AnisotropicSmoothing", "separable_gaussian", "CAD/GAD 代表：可分离高斯", false },
    { 2, "AntiAlias", "box_mean_r4", "小半径盒滤波", false },
    { 3, "BiasCorrection", "pointwise_scale", "强度缩放代表", false },
    { 4, "BinaryMathematicalMorphology", "skip", "二值形态学，非 float 热点", true },
    { 5, "Colormap", "pointwise_scale", "查找表≈点运算", false },
    { 6, "Convolution", "separable_gaussian", "空间卷积代表", false },
    { 7, "CurvatureFlow", "separable_gaussian", "PDE 类代表", false },
    { 8, "Deconvolution", "separable_gaussian", "卷积型去卷积代表", false },
    { 9, "Denoising", "separable_gaussian", "高斯去噪", false },
    { 10, "DiffusionTensorImage", "gradient_mag", "张量场梯度代表", false },
    { 11, "DisplacementField", "pointwise_scale", "向量场点运算代表", false },
    { 12, "DistanceMap", "box_mean_r4", "距离图≈邻域传播代表", false },
    { 13, "FastMarching", "sum_scan", "前沿传播≈全图扫描代表", false },
    { 14, "FFT", "skip", "FFTW 仅 float/double，不做 FP16 存储", true },
    { 15, "GPUAnisotropicSmoothing", "separable_gaussian", "CPU 等价 #1", false },
    { 16, "GPUImageFilterBase", "skip", "基类", true },
    { 17, "GPUSmoothing", "box_mean", "CPU 等价 Smoothing", false },
    { 18, "GPUThresholding", "pointwise_scale", "CPU 等价 Thresholding", false },
    { 19, "ImageCompare", "sum_scan", "像素比较≈遍历", false },
    { 20, "ImageCompose", "pointwise_scale", "通道合成", false },
    { 21, "ImageFeature", "bilateral_r4", "Bilateral 代表", false },
    { 22, "ImageFilterBase", "skip", "基类", true },
    { 23, "ImageFrequency", "skip", "频域 FFT", true },
    { 24, "ImageFusion", "pointwise_scale", "加权融合≈点运算", false },
    { 25, "ImageGradient", "gradient_mag", "梯度幅值", false },
    { 26, "ImageGrid", "box_mean_r4", "重采样≈邻域插值代表", false },
    { 27, "ImageIntensity", "pointwise_scale", "强度运算", false },
    { 28, "ImageLabel", "skip", "标签图 uint，非 FP16 场景", true },
    { 29, "ImageNoise", "pointwise_scale", "加噪声≈点运算", false },
    { 30, "ImageSources", "pointwise_scale", "生成源≈填充", false },
    { 31, "ImageStatistics", "sum_scan", "全图统计", false },
    { 32, "LabelMap", "skip", "标签图", true },
    { 33, "MathematicalMorphology", "median3", "中值滤波", false },
    { 34, "Path", "gradient_mag", "轮廓≈梯度", false },
    { 35, "QuadEdgeMeshFiltering", "skip", "网格，无体素 FP16 代表", true },
    { 36, "Smoothing", "box_mean", "BoxMean radius=15", false },
    { 37, "SpatialFunction", "skip", "基类", true },
    { 38, "Thresholding", "pointwise_scale", "Otsu 后处理≈点运算", false },
  };

  std::cout << "=== fp16_modules_sweep_bench ===\n";
  std::cout << "method=FP16_storage+static_cast_widen+FP32_compute (+fp16, NOT fmlal)\n";
  std::cout << "size=" << w << 'x' << h << " runs=" << runs << " threads=" << threads
            << " omp_max=" << omp_get_max_threads() << '\n';
  std::cout << std::fixed << std::setprecision(4);
  std::cout << "\n# id|module|kernel|ms_f32|ms_f16|speedup|max_abs|status|note\n";

  std::vector<BenchRow> rows;

  for (const ModuleCase & m : modules)
  {
    BenchRow row;
    row.id = m.id;
    row.name = m.name;
    row.kernel = m.kernel;
    row.note = m.note;

    if (m.skip)
    {
      row.status = "N/A";
      rows.push_back(row);
      std::cout << m.id << '|' << m.name << '|' << m.kernel << "|—|—|—|—|N/A|" << m.note << '\n';
      continue;
    }

    const std::string k = m.kernel;
    if (k == "box_mean" || k == "box_mean_r4")
    {
      const int r = (k == "box_mean") ? radius : 4;
      RunPair(
        row,
        runs,
        [&](std::vector<float> & o) { BoxMeanF32(imgF, o, w, h, r); },
        [&](std::vector<float> & o) { BoxMeanF16(imgH, o, w, h, r); },
        imgF);
    }
    else if (k == "separable_gaussian")
    {
      RunPair(
        row,
        runs,
        [&](std::vector<float> & o) { SeparableGaussianF32(imgF, tmpF32, o, w, h, radius, sigma); },
        [&](std::vector<float> & o) { SeparableGaussianF16(imgH, tmpF32, o, w, h, radius, sigma); },
        imgF);
    }
    else if (k == "pointwise_scale")
    {
      RunPair(
        row,
        runs,
        [&](std::vector<float> & o) { PointwiseScaleF32(imgF, o, 1.02f); },
        [&](std::vector<float> & o) { PointwiseScaleF16(imgH, o, 1.02f); },
        imgF);
    }
    else if (k == "gradient_mag")
    {
      RunPair(
        row,
        runs,
        [&](std::vector<float> & o) { GradientMagF32(imgF, o, w, h); },
        [&](std::vector<float> & o) { GradientMagF16(imgH, o, w, h); },
        imgF);
    }
    else if (k == "median3")
    {
      RunPair(
        row,
        runs,
        [&](std::vector<float> & o) { Median3F32(imgF, o, w, h); },
        [&](std::vector<float> & o) { Median3F16(imgH, o, w, h); },
        imgF);
    }
    else if (k == "sum_scan")
    {
      RunPair(
        row,
        runs,
        [&](std::vector<float> & o) {
          const float s = SumF32(imgF);
          o.assign(1, s);
        },
        [&](std::vector<float> & o) {
          const float s = SumF16(imgH);
          o.assign(1, s);
        },
        imgF);
      row.maxAbs = std::fabs(SumF32(imgF) - SumF16(imgH));
    }
    else if (k == "bilateral_r4")
    {
      const int br = 4;
      RunPair(
        row,
        runs,
        [&](std::vector<float> & o) { BilateralDomainF32(imgF, o, w, h, br, 0.f); },
        [&](std::vector<float> & o) { BilateralDomainF16(imgH, o, w, h, br); },
        imgF);
    }
    else
    {
      row.status = "UNKNOWN";
    }

    rows.push_back(row);
    std::cout << row.id << '|' << row.name << '|' << row.kernel << '|' << row.msF32 << '|' << row.msF16 << '|'
              << row.speedup << '|' << std::setprecision(6) << row.maxAbs << std::setprecision(4) << '|' << row.status
              << '|' << row.note << '\n';
  }

  // Registration / Segmentation 代表
  std::cout << "\n# --- Registration / Segmentation ---\n";
  {
    std::vector<float16_t> imgH2 = imgH;
    for (size_t i = 0; i < imgH2.size(); i += 97)
    {
      imgH2[i] = vcvt_f16_f32(vdupq_n_f32(static_cast<float>(imgF[i]) + 3.f))[0];
    }
    BenchRow row;
    row.id = 101;
    row.name = "Metricsv4";
    row.kernel = "local_mse_r4";
    row.note = "局部 MSE 代表";
    RunPair(
      row,
      runs,
      [&](std::vector<float> & o) { LocalMseF32(imgF, imgF, o, w, h, 4); },
      [&](std::vector<float> & o) { LocalMseF16(imgH, imgH2, o, w, h, 4); },
      imgF);
    std::cout << row.id << '|' << row.name << '|' << row.kernel << '|' << row.msF32 << '|' << row.msF16 << '|'
              << row.speedup << '|' << std::setprecision(6) << row.maxAbs << std::setprecision(4) << "|OK|"
              << row.note << '\n';
  }
  for (const char * seg : { "RegistrationMethodsv4", "LevelSets", "Watersheds", "ConnectedComponents" })
  {
    BenchRow row;
    row.id = 102;
    row.name = seg;
    row.kernel = "separable_gaussian";
    row.note = "分割/配准 pipeline 代表";
    RunPair(
      row,
      runs,
      [&](std::vector<float> & o) { SeparableGaussianF32(imgF, tmpF32, o, w, h, radius, sigma); },
      [&](std::vector<float> & o) { SeparableGaussianF16(imgH, tmpF32, o, w, h, radius, sigma); },
      imgF);
    std::cout << "102|" << seg << "|separable_gaussian|" << row.msF32 << '|' << row.msF16 << '|' << row.speedup
              << '|' << std::setprecision(6) << row.maxAbs << std::setprecision(4) << "|OK|" << row.note << '\n';
  }

  // 汇总
  double sumSp = 0;
  int    cntSp = 0;
  for (const BenchRow & r : rows)
  {
    if (r.status == "OK")
    {
      sumSp += r.speedup;
      ++cntSp;
    }
  }
  std::cout << "\n# SUMMARY tested=" << cntSp << " avg_speedup=" << (cntSp ? sumSp / cntSp : 0) << '\n';
  return 0;
}
