// itk_hybrid_precision_demo — 推荐 pipeline：全程 Image<float>。
// 对照 full_double 仅用于精度验证；cast 链 hybrid 已废弃（见 混合精度.md）。

#include "itkCastImageFilter.h"
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkMedianImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include <cmath>
#include <iomanip>
#include <iostream>

int
main(int, char **)
{
  constexpr unsigned int Dim = 2;
  using FloatImageType = itk::Image<float, Dim>;
  using DoubleImageType = itk::Image<double, Dim>;

  auto input = FloatImageType::New();
  {
    FloatImageType::SizeType   size;
    FloatImageType::IndexType  index;
    FloatImageType::RegionType region;
    size.Fill(256);
    index.Fill(0);
    region.SetSize(size);
    region.SetIndex(index);
    input->SetRegions(region);
    input->Allocate();
    itk::ImageRegionIterator<FloatImageType> it(input, region);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      const auto         idx = it.GetIndex();
      const unsigned int x = static_cast<unsigned int>(idx[0]);
      const unsigned int y = static_cast<unsigned int>(idx[1]);
      const float        v = ((x / 32) + (y / 32)) % 2 == 0 ? 0.0F : 1000.0F;
      it.Set(v);
    }
  }

  using MedianF = itk::MedianImageFilter<FloatImageType, FloatImageType>;
  auto median = MedianF::New();
  {
    MedianF::InputSizeType r;
    r.Fill(1);
    median->SetRadius(r);
  }
  median->SetInput(input);

  // 推荐：全程 float
  using SmoothF = itk::SmoothingRecursiveGaussianImageFilter<FloatImageType, FloatImageType>;
  auto smoothF = SmoothF::New();
  smoothF->SetInput(median->GetOutput());
  smoothF->SetSigma(2.0);

  // 精度参考：full_double（非生产推荐路径）
  using CastToDouble = itk::CastImageFilter<FloatImageType, DoubleImageType>;
  auto castUp = CastToDouble::New();
  castUp->SetInput(median->GetOutput());
  using SmoothD = itk::SmoothingRecursiveGaussianImageFilter<DoubleImageType, DoubleImageType>;
  auto smoothD = SmoothD::New();
  smoothD->SetInput(castUp->GetOutput());
  smoothD->SetSigma(2.0);

  smoothF->Update();
  smoothD->Update();

  FloatImageType::Pointer  floatOut = smoothF->GetOutput();
  DoubleImageType::Pointer doubleOut = smoothD->GetOutput();

  double        sumAbs = 0.0;
  double        maxAbs = 0.0;
  unsigned long count = 0;
  itk::ImageRegionConstIterator<DoubleImageType> itD(doubleOut, doubleOut->GetBufferedRegion());
  itk::ImageRegionConstIterator<FloatImageType>  itF(floatOut, floatOut->GetBufferedRegion());
  for (itD.GoToBegin(), itF.GoToBegin(); !itD.IsAtEnd(); ++itD, ++itF)
  {
    const double d = std::fabs(itD.Get() - static_cast<double>(itF.Get()));
    sumAbs += d;
    maxAbs = std::max(maxAbs, d);
    ++count;
  }
  const double mae = count > 0 ? sumAbs / static_cast<double>(count) : 0.0;

  std::cout << std::setprecision(12);
  std::cout << "itk_hybrid_precision_demo: recommended=full_float (Median+Gaussian)\n";
  std::cout << "precision check vs full_double: max_abs=" << maxAbs << " mean_abs=" << mae << '\n';
  std::cout << "note: cast-chain hybrid removed; use precision_pipeline_bench for benchmarks\n";

  return 0;
}
