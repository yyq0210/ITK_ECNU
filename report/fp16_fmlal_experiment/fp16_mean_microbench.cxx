// FP16 / FMLAL POC：模拟 ITK Mean / 1D Gaussian 卷积内核
// 用法: fp16_mean_microbench [width] [height] [radius] [runs] [threads]
#include <arm_neon.h>
#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

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
      const float v = 200.f * std::sin(0.01f * x) * std::cos(0.013f * y) + 400.f + 0.3f * x;
      img[static_cast<size_t>(y) * w + x] = v;
    }
  }
}

static void
FloatToHalfImage(const std::vector<float> & src, std::vector<float16_t> & dst)
{
  dst.resize(src.size());
  size_t i = 0;
  for (; i + 8 <= src.size(); i += 8)
  {
    const float32x4_t a = vld1q_f32(src.data() + i);
    const float32x4_t b = vld1q_f32(src.data() + i + 4);
    vst1q_f16(dst.data() + i, vcombine_f16(vcvt_f16_f32(a), vcvt_f16_f32(b)));
  }
  for (; i < src.size(); ++i)
  {
    dst[i] = vcvt_f16_f32(vdupq_n_f32(src[i]))[0];
  }
}

static double
MaxAbsDiff(const std::vector<float> & a, const std::vector<float> & b)
{
  double m = 0.0;
  for (size_t i = 0; i < a.size(); ++i)
  {
    m = std::max(m, static_cast<double>(std::fabs(a[i] - b[i])));
  }
  return m;
}

// --- Box mean: FP32 存储 ---
static void
BoxMeanF32(const std::vector<float> & in, std::vector<float> & out, int w, int h, int radius)
{
  const int    diam = 2 * radius + 1;
  const float  invN = 1.f / static_cast<float>(diam * diam);
  const size_t n = out.size();
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        const int yy = std::min(h - 1, std::max(0, y + dy));
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int xx = std::min(w - 1, std::max(0, x + dx));
          sum += in[static_cast<size_t>(yy) * w + xx];
        }
      }
      out[static_cast<size_t>(y) * w + x] = sum * invN;
    }
  }
  (void)n;
}

// --- Box mean: FP16 存储，load 后 fcvtl 到 FP32 再累加 ---
static void
BoxMeanF16Widen(const std::vector<float16_t> & in, std::vector<float> & out, int w, int h, int radius)
{
  const int   diam = 2 * radius + 1;
  const float invN = 1.f / static_cast<float>(diam * diam);
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        const int yy = std::min(h - 1, std::max(0, y + dy));
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int           xx = std::min(w - 1, std::max(0, x + dx));
          const float16_t     v = in[static_cast<size_t>(yy) * w + xx];
          sum += static_cast<float>(v);
        }
      }
      out[static_cast<size_t>(y) * w + x] = sum * invN;
    }
  }
}

static void
GaussianRowF16Widen(const std::vector<float16_t> & in,
                    std::vector<float> &             out,
                    int                              w,
                    int                              h,
                    int                              radius,
                    const std::vector<float> &       weights)
{
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int k = -radius; k <= radius; ++k)
      {
        const int       xx = std::min(w - 1, std::max(0, x + k));
        const float16_t pv = in[static_cast<size_t>(y) * w + xx];
        sum += static_cast<float>(pv) * weights[static_cast<size_t>(k + radius)];
      }
      out[static_cast<size_t>(y) * w + x] = sum;
    }
  }
}

#ifdef USE_FP16FML
// --- 1D Gaussian 行卷积：FP16 像素 × FP16 权重，FMLAL 累加到 FP32 ---
static void
MakeGaussianHalfWeights(int radius, float sigma, std::vector<float16_t> & w)
{
  const int   n = 2 * radius + 1;
  const float s2 = 2.f * sigma * sigma;
  std::vector<float> wf(n);
  float              sum = 0.f;
  for (int i = 0; i < n; ++i)
  {
    const float d = static_cast<float>(i - radius);
    wf[i] = std::exp(-(d * d) / s2);
    sum += wf[i];
  }
  w.resize(n);
  for (int i = 0; i < n; ++i)
  {
    w[i] = vcvt_f16_f32(vdupq_n_f32(wf[i] / sum))[0];
  }
}

static void
GaussianRowF16Fmlal(const std::vector<float16_t> & in,
                    std::vector<float> &             out,
                    int                              w,
                    int                              h,
                    int                              radius,
                    const std::vector<float16_t> &   weights)
{
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float32x4_t acc0 = vdupq_n_f32(0.f);
      float32x4_t acc1 = vdupq_n_f32(0.f);
      for (int k = -radius; k <= radius; ++k)
      {
        const int       xx = std::min(w - 1, std::max(0, x + k));
        const float16_t pv = in[static_cast<size_t>(y) * w + xx];
        const float16_t wv = weights[static_cast<size_t>(k + radius)];
        const float16x8_t vp = vdupq_n_f16(pv);
        const float16x8_t ww = vdupq_n_f16(wv);
        acc0 = vfmlalq_low_f16(acc0, vp, ww);
        acc1 = vfmlalq_high_f16(acc1, vp, ww);
      }
      out[static_cast<size_t>(y) * w + x] = vaddvq_f32(vaddq_f32(acc0, acc1));
    }
  }
}
#endif

static void
GaussianRowF32(const std::vector<float> & in,
               std::vector<float> &       out,
               int                        w,
               int                        h,
               int                        radius,
               const std::vector<float> & weights)
{
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int k = -radius; k <= radius; ++k)
      {
        const int xx = std::min(w - 1, std::max(0, x + k));
        sum += in[static_cast<size_t>(y) * w + xx] * weights[static_cast<size_t>(k + radius)];
      }
      out[static_cast<size_t>(y) * w + x] = sum;
    }
  }
}

static void
MakeGaussianFloatWeights(int radius, float sigma, std::vector<float> & w)
{
  const int   n = 2 * radius + 1;
  const float s2 = 2.f * sigma * sigma;
  w.resize(n);
  float sum = 0.f;
  for (int i = 0; i < n; ++i)
  {
    const float d = static_cast<float>(i - radius);
    w[i] = std::exp(-(d * d) / s2);
    sum += w[i];
  }
  for (float & v : w)
  {
    v /= sum;
  }
}

template <typename Fn>
static double
BenchMs(Fn fn, int runs)
{
  fn(); // warmup
  double total = 0.0;
  for (int r = 0; r < runs; ++r)
  {
    const auto t0 = Clock::now();
    fn();
    total += std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  }
  return total / runs;
}

int
main(int argc, char ** argv)
{
  const int w = ParseInt(argc, argv, 1, 1024);
  const int h = ParseInt(argc, argv, 2, 1024);
  const int radius = ParseInt(argc, argv, 3, 15);
  const int runs = ParseInt(argc, argv, 4, 5);
  const int threads = ParseInt(argc, argv, 5, 96);

  omp_set_num_threads(threads);

  std::vector<float>      imgF;
  std::vector<float16_t>  imgH;
  std::vector<float>      outF32, outF16, outGaussF32, outGaussF16, outGaussF16W;
  FillSynthetic(imgF, w, h);
  FloatToHalfImage(imgF, imgH);
  outF32.resize(imgF.size());
  outF16.resize(imgF.size());
  outGaussF32.resize(imgF.size());
  outGaussF16.resize(imgF.size());
  outGaussF16W.resize(imgF.size());

  std::vector<float> gaussW;
  MakeGaussianFloatWeights(radius, static_cast<float>(radius) / 3.f, gaussW);

  std::cout << "=== fp16_mean_microbench ===\n";
  std::cout << "size=" << w << "x" << h << " radius=" << radius << " runs=" << runs
            << " threads=" << threads << " omp_max=" << omp_get_max_threads() << '\n';
#ifdef USE_FP16FML
  std::cout << "build: USE_FP16FML=1 (FMLAL intrinsic path enabled)\n";
#else
  std::cout << "build: USE_FP16FML=0 (no FMLAL binary)\n";
#endif
  std::cout << std::fixed << std::setprecision(4);

  const double msBoxF32 =
    BenchMs([&]() { BoxMeanF32(imgF, outF32, w, h, radius); }, runs);
  const double msBoxF16 =
    BenchMs([&]() { BoxMeanF16Widen(imgH, outF16, w, h, radius); }, runs);
  const double maxAbsBox = MaxAbsDiff(outF32, outF16);

  std::cout << "\n--- BoxMean (31x31 when radius=15) ---\n";
  std::cout << "F32_storage          ms=" << msBoxF32 << '\n';
  std::cout << "F16_storage+FCVT+FP32 ms=" << msBoxF16 << " speedup_vs_f32=" << (msBoxF32 / msBoxF16)
            << "x max_abs=" << std::setprecision(6) << maxAbsBox << std::setprecision(4) << '\n';

  const double msGaussF32 =
    BenchMs([&]() { GaussianRowF32(imgF, outGaussF32, w, h, radius, gaussW); }, runs);

  std::cout << "\n--- Gaussian 1D row conv (sigma=radius/3) ---\n";
  std::cout << "F32 MAC              ms=" << msGaussF32 << '\n';

  const double msGaussF16W = BenchMs(
    [&]() { GaussianRowF16Widen(imgH, outGaussF16W, w, h, radius, gaussW); }, runs);
  const double maxAbsGaussW = MaxAbsDiff(outGaussF32, outGaussF16W);
  std::cout << "F16+FCVT+FP32 MAC    ms=" << msGaussF16W << " speedup_vs_f32=" << (msGaussF32 / msGaussF16W)
            << "x max_abs=" << std::setprecision(6) << maxAbsGaussW << std::setprecision(4) << '\n';

#ifdef USE_FP16FML
  std::vector<float16_t> gaussWH;
  MakeGaussianHalfWeights(radius, static_cast<float>(radius) / 3.f, gaussWH);
  const double msGaussFmlal = BenchMs(
    [&]() { GaussianRowF16Fmlal(imgH, outGaussF16, w, h, radius, gaussWH); }, runs);
  const double maxAbsGauss = MaxAbsDiff(outGaussF32, outGaussF16);
  std::cout << "F16+FMLAL MAC        ms=" << msGaussFmlal << " speedup_vs_f32=" << (msGaussF32 / msGaussFmlal)
            << "x max_abs=" << std::setprecision(6) << maxAbsGauss << std::setprecision(4) << '\n';
#else
  std::cout << "F16+FMLAL MAC        skipped (not compiled with +fp16fml)\n";
#endif

  return 0;
}
