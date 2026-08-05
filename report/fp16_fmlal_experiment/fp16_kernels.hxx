#pragma once
// FP16 存储 + 拓宽到 FP32 累加的通用内核（+fp16，无 FMLAL）
#include <arm_neon.h>
#include <omp.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

inline void
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

inline double
MaxAbsDiff(const std::vector<float> & a, const std::vector<float> & b)
{
  double m = 0.0;
  const size_t n = std::min(a.size(), b.size());
  for (size_t i = 0; i < n; ++i)
  {
    m = std::max(m, static_cast<double>(std::fabs(a[i] - b[i])));
  }
  return m;
}

inline int
Clamped(int v, int lo, int hi)
{
  return std::min(hi, std::max(lo, v));
}

// --- Box mean ---
inline void
BoxMeanF32(const std::vector<float> & in, std::vector<float> & out, int w, int h, int radius)
{
  const int   diam = 2 * radius + 1;
  const float invN = 1.f / static_cast<float>(diam * diam);
  out.resize(in.size());
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        const int yy = Clamped(y + dy, 0, h - 1);
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int xx = Clamped(x + dx, 0, w - 1);
          sum += in[static_cast<size_t>(yy) * w + xx];
        }
      }
      out[static_cast<size_t>(y) * w + x] = sum * invN;
    }
  }
}

inline void
BoxMeanF16(const std::vector<float16_t> & in, std::vector<float> & out, int w, int h, int radius)
{
  const int   diam = 2 * radius + 1;
  const float invN = 1.f / static_cast<float>(diam * diam);
  out.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        const int yy = Clamped(y + dy, 0, h - 1);
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int       xx = Clamped(x + dx, 0, w - 1);
          const float16_t v = in[static_cast<size_t>(yy) * w + xx];
          sum += static_cast<float>(v);
        }
      }
      out[static_cast<size_t>(y) * w + x] = sum * invN;
    }
  }
}

// --- 1D Gaussian 行卷积（可分离高斯代表） ---
inline void
MakeGaussianWeights(int radius, float sigma, std::vector<float> & w)
{
  const int n = 2 * radius + 1;
  w.resize(n);
  const float s2 = 2.f * sigma * sigma;
  float       sum = 0.f;
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

inline void
ConvRowF32(const std::vector<float> & in,
           std::vector<float> &       out,
           int                        w,
           int                        h,
           int                        radius,
           const std::vector<float> & weights)
{
  out.resize(in.size());
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int k = -radius; k <= radius; ++k)
      {
        const int xx = Clamped(x + k, 0, w - 1);
        sum += in[static_cast<size_t>(y) * w + xx] * weights[static_cast<size_t>(k + radius)];
      }
      out[static_cast<size_t>(y) * w + x] = sum;
    }
  }
}

inline void
ConvRowF16(const std::vector<float16_t> & in,
           std::vector<float> &           out,
           int                            w,
           int                            h,
           int                            radius,
           const std::vector<float> &     weights)
{
  out.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float sum = 0.f;
      for (int k = -radius; k <= radius; ++k)
      {
        const int       xx = Clamped(x + k, 0, w - 1);
        const float16_t pv = in[static_cast<size_t>(y) * w + xx];
        sum += static_cast<float>(pv) * weights[static_cast<size_t>(k + radius)];
      }
      out[static_cast<size_t>(y) * w + x] = sum;
    }
  }
}

inline void
SeparableGaussianF32(const std::vector<float> & in,
                     std::vector<float> &       tmp,
                     std::vector<float> &       out,
                     int                        w,
                     int                        h,
                     int                        radius,
                     float                      sigma)
{
  std::vector<float> weights;
  MakeGaussianWeights(radius, sigma, weights);
  ConvRowF32(in, tmp, w, h, radius, weights);
  // 列方向：转置思路 — 按列做行卷积
  out = tmp;
#pragma omp parallel for schedule(static)
  for (int x = 0; x < w; ++x)
  {
    for (int y = 0; y < h; ++y)
    {
      float sum = 0.f;
      for (int k = -radius; k <= radius; ++k)
      {
        const int yy = Clamped(y + k, 0, h - 1);
        sum += tmp[static_cast<size_t>(yy) * w + x] * weights[static_cast<size_t>(k + radius)];
      }
      out[static_cast<size_t>(y) * w + x] = sum;
    }
  }
}

inline void
SeparableGaussianF16(const std::vector<float16_t> & in,
                     std::vector<float> &             tmp,
                     std::vector<float> &             out,
                     int                              w,
                     int                              h,
                     int                              radius,
                     float                            sigma)
{
  std::vector<float> weights;
  MakeGaussianWeights(radius, sigma, weights);
  ConvRowF16(in, tmp, w, h, radius, weights);
  out = tmp;
#pragma omp parallel for schedule(static)
  for (int x = 0; x < w; ++x)
  {
    for (int y = 0; y < h; ++y)
    {
      float sum = 0.f;
      for (int k = -radius; k <= radius; ++k)
      {
        const int yy = Clamped(y + k, 0, h - 1);
        sum += tmp[static_cast<size_t>(yy) * w + x] * weights[static_cast<size_t>(k + radius)];
      }
      out[static_cast<size_t>(y) * w + x] = sum;
    }
  }
}

// --- 点运算（ImageIntensity / Compose / Fusion 代表） ---
inline void
PointwiseScaleF32(const std::vector<float> & in, std::vector<float> & out, float scale)
{
  out.resize(in.size());
#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < in.size(); ++i)
  {
    out[i] = in[i] * scale + 0.1f;
  }
}

inline void
PointwiseScaleF16(const std::vector<float16_t> & in, std::vector<float> & out, float scale)
{
  out.resize(in.size());
#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < in.size(); ++i)
  {
    out[i] = static_cast<float>(in[i]) * scale + 0.1f;
  }
}

// --- 梯度幅值（ImageGradient 代表） ---
inline void
GradientMagF32(const std::vector<float> & in, std::vector<float> & out, int w, int h)
{
  out.resize(in.size());
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      const float cx = in[static_cast<size_t>(y) * w + Clamped(x + 1, 0, w - 1)] -
                       in[static_cast<size_t>(y) * w + Clamped(x - 1, 0, w - 1)];
      const float cy = in[static_cast<size_t>(Clamped(y + 1, 0, h - 1)) * w + x] -
                       in[static_cast<size_t>(Clamped(y - 1, 0, h - 1)) * w + x];
      out[static_cast<size_t>(y) * w + x] = std::sqrt(cx * cx + cy * cy);
    }
  }
}

inline void
GradientMagF16(const std::vector<float16_t> & in, std::vector<float> & out, int w, int h)
{
  out.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      const float cx = static_cast<float>(in[static_cast<size_t>(y) * w + Clamped(x + 1, 0, w - 1)]) -
                       static_cast<float>(in[static_cast<size_t>(y) * w + Clamped(x - 1, 0, w - 1)]);
      const float cy =
        static_cast<float>(in[static_cast<size_t>(Clamped(y + 1, 0, h - 1)) * w + x]) -
        static_cast<float>(in[static_cast<size_t>(Clamped(y - 1, 0, h - 1)) * w + x]);
      out[static_cast<size_t>(y) * w + x] = std::sqrt(cx * cx + cy * cy);
    }
  }
}

// --- 3×3 中值（MathematicalMorphology / Median 代表） ---
inline void
Median3F32(const std::vector<float> & in, std::vector<float> & out, int w, int h)
{
  out.resize(in.size());
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float buf[9];
      int   n = 0;
      for (int dy = -1; dy <= 1; ++dy)
      {
        for (int dx = -1; dx <= 1; ++dx)
        {
          buf[n++] = in[static_cast<size_t>(Clamped(y + dy, 0, h - 1)) * w + Clamped(x + dx, 0, w - 1)];
        }
      }
      std::nth_element(buf, buf + 4, buf + 9);
      out[static_cast<size_t>(y) * w + x] = buf[4];
    }
  }
}

inline void
Median3F16(const std::vector<float16_t> & in, std::vector<float> & out, int w, int h)
{
  out.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float buf[9];
      int   n = 0;
      for (int dy = -1; dy <= 1; ++dy)
      {
        for (int dx = -1; dx <= 1; ++dx)
        {
          buf[n++] =
            static_cast<float>(in[static_cast<size_t>(Clamped(y + dy, 0, h - 1)) * w + Clamped(x + dx, 0, w - 1)]);
        }
      }
      std::nth_element(buf, buf + 4, buf + 9);
      out[static_cast<size_t>(y) * w + x] = buf[4];
    }
  }
}

// --- 全图求和（ImageStatistics 代表） ---
inline float
SumF32(const std::vector<float> & in)
{
  double sum = 0.0;
#pragma omp parallel for reduction(+ : sum) schedule(static)
  for (size_t i = 0; i < in.size(); ++i)
  {
    sum += in[i];
  }
  return static_cast<float>(sum);
}

inline float
SumF16(const std::vector<float16_t> & in)
{
  double sum = 0.0;
#pragma omp parallel for reduction(+ : sum) schedule(static)
  for (size_t i = 0; i < in.size(); ++i)
  {
    sum += static_cast<float>(in[i]);
  }
  return static_cast<float>(sum);
}

// --- 简化的 Bilateral 域核卷积（ImageFeature 代表，range 表预计算） ---
inline void
BilateralDomainF32(const std::vector<float> & in,
                   std::vector<float> &       out,
                   int                        w,
                   int                        h,
                   int                        radius,
                   float                      center)
{
  std::vector<float> spatial;
  MakeGaussianWeights(radius, static_cast<float>(radius) / 2.f, spatial);
  out.resize(in.size());
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      const float centerVal = in[static_cast<size_t>(y) * w + x];
      float       sum = 0.f;
      float       wsum = 0.f;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int   yy = Clamped(y + dy, 0, h - 1);
          const int   xx = Clamped(x + dx, 0, w - 1);
          const float pv = in[static_cast<size_t>(yy) * w + xx];
          const float sw = spatial[static_cast<size_t>(dy + radius)] * spatial[static_cast<size_t>(dx + radius)];
          const float rw = std::exp(-std::fabs(pv - centerVal) / 50.f);
          const float wgt = sw * rw;
          sum += pv * wgt;
          wsum += wgt;
        }
      }
      out[static_cast<size_t>(y) * w + x] = sum / std::max(wsum, 1e-6f);
    }
  }
  (void)center;
}

inline void
BilateralDomainF16(const std::vector<float16_t> & in,
                   std::vector<float> &             out,
                   int                              w,
                   int                              h,
                   int                              radius)
{
  std::vector<float> spatial;
  MakeGaussianWeights(radius, static_cast<float>(radius) / 2.f, spatial);
  out.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      const float centerVal = static_cast<float>(in[static_cast<size_t>(y) * w + x]);
      float       sum = 0.f;
      float       wsum = 0.f;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int   yy = Clamped(y + dy, 0, h - 1);
          const int   xx = Clamped(x + dx, 0, w - 1);
          const float pv = static_cast<float>(in[static_cast<size_t>(yy) * w + xx]);
          const float sw = spatial[static_cast<size_t>(dy + radius)] * spatial[static_cast<size_t>(dx + radius)];
          const float rw = std::exp(-std::fabs(pv - centerVal) / 50.f);
          const float wgt = sw * rw;
          sum += pv * wgt;
          wsum += wgt;
        }
      }
      out[static_cast<size_t>(y) * w + x] = sum / std::max(wsum, 1e-6f);
    }
  }
}

// --- 局部 MSE 块（Metricsv4 代表） ---
inline void
LocalMseF32(const std::vector<float> & fixed,
            const std::vector<float> & moving,
            std::vector<float> &       out,
            int                        w,
            int                        h,
            int                        radius)
{
  out.resize(fixed.size());
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float err = 0.f;
      int   cnt = 0;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int yy = Clamped(y + dy, 0, h - 1);
          const int xx = Clamped(x + dx, 0, w - 1);
          const float d = fixed[static_cast<size_t>(yy) * w + xx] - moving[static_cast<size_t>(yy) * w + xx];
          err += d * d;
          ++cnt;
        }
      }
      out[static_cast<size_t>(y) * w + x] = err / static_cast<float>(cnt);
    }
  }
}

inline void
LocalMseF16(const std::vector<float16_t> & fixed,
            const std::vector<float16_t> & moving,
            std::vector<float> &           out,
            int                            w,
            int                            h,
            int                            radius)
{
  out.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
#pragma omp parallel for schedule(static)
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      float err = 0.f;
      int   cnt = 0;
      for (int dy = -radius; dy <= radius; ++dy)
      {
        for (int dx = -radius; dx <= radius; ++dx)
        {
          const int yy = Clamped(y + dy, 0, h - 1);
          const int xx = Clamped(x + dx, 0, w - 1);
          const float d = static_cast<float>(fixed[static_cast<size_t>(yy) * w + xx]) -
                          static_cast<float>(moving[static_cast<size_t>(yy) * w + xx]);
          err += d * d;
          ++cnt;
        }
      }
      out[static_cast<size_t>(y) * w + x] = err / static_cast<float>(cnt);
    }
  }
}
