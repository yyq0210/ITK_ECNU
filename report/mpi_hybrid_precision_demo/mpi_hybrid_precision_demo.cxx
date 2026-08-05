// MPI + 真实体数据（ImageFileReader<float,3>）+ 高斯平滑 float vs double 对照。
// 注意：cast 链 hybrid 已废弃；生产路径请用全程 Image<float>（见 操作记录_混合精度float化实验报告_2026-06-10.md）。
// 本程序保留 hybrid/double 支路仅作 MPI 误差归约对照。
// 沿「当前体积最长的一维」划分子块（薄层体积避免沿层厚方向切得过细），必要时对子块再 pad，
// 以满足 SmoothingRecursiveGaussian 各维 ≥4 像素的要求。
//
// 用法（在第一台发起多机时，各节点须能访问同一文件路径，参见文档）：
//   mpirun ... ./mpi_hybrid_precision_demo /path/to/volume.mha [sigma]
//
// 默认 sigma=1.5；可通过 argv[2] 覆盖。

#include <mpi.h>

#include "itkCastImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionConstIterator.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{

constexpr unsigned int Dimension = 3;
using FloatImageType = itk::Image<float, Dimension>;
using DoubleImageType = itk::Image<double, Dimension>;

double
ParseSigma(int argc, char ** argv)
{
  double sigma = 1.5;
  if (argc >= 3)
  {
    sigma = std::strtod(argv[2], nullptr);
    if (!(sigma > 0.0))
    {
      sigma = 1.5;
    }
  }
  return sigma;
}

} // namespace

int
main(int argc, char ** argv)
{
  MPI_Init(&argc, &argv);

  int rank = 0;
  int mpi_size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  if (argc < 2)
  {
    if (rank == 0)
    {
      std::cerr << "用法: " << (argc > 0 ? argv[0] : "mpi_hybrid_precision_demo")
                << " <体数据路径.mha|.mhd|.png...> [sigma]\n";
    }
    MPI_Finalize();
    return 1;
  }

  const std::string filename(argv[1]);
  const double      sigma = ParseSigma(argc, argv);

  try
  {
    using ReaderType = itk::ImageFileReader<FloatImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(filename);
    reader->Update();
    FloatImageType::Pointer volume = reader->GetOutput();

    // SmoothingRecursiveGaussianImageFilter 要求各维像素数 ≥ 4；薄体积（如仅 3 层切片）需先 pad。
    {
      auto                          sz = volume->GetLargestPossibleRegion().GetSize();
      FloatImageType::SizeType      lowerPad;
      FloatImageType::SizeType      upperPad;
      constexpr itk::SizeValueType kMinAxis = 4;
      lowerPad.Fill(0);
      upperPad.Fill(0);
      for (unsigned int d = 0; d < Dimension; ++d)
      {
        if (sz[d] < kMinAxis)
        {
          upperPad[d] = kMinAxis - sz[d];
        }
      }
      const bool needPad = [&]() {
        for (unsigned int d = 0; d < Dimension; ++d)
        {
          if (upperPad[d] != 0)
          {
            return true;
          }
        }
        return false;
      }();
      if (needPad)
      {
        using PadType = itk::ConstantPadImageFilter<FloatImageType, FloatImageType>;
        auto pad = PadType::New();
        pad->SetInput(volume);
        pad->SetPadLowerBound(lowerPad);
        pad->SetPadUpperBound(upperPad);
        pad->SetConstant(0.0f);
        pad->Update();
        volume = pad->GetOutput();
      }
    }

    const auto          inputRegion = volume->GetLargestPossibleRegion();
    auto                inputSize = inputRegion.GetSize();
    unsigned int        axis = 0;
    itk::SizeValueType  longest = inputSize[0];
    for (unsigned int d = 1; d < Dimension; ++d)
    {
      if (inputSize[d] > longest)
      {
        longest = inputSize[d];
        axis = d;
      }
    }
    const unsigned long extent = static_cast<unsigned long>(inputSize[axis]);

    const unsigned long z0 = (static_cast<unsigned long>(rank) * extent) / static_cast<unsigned long>(mpi_size);
    const unsigned long z1 =
      (static_cast<unsigned long>(rank + 1) * extent) / static_cast<unsigned long>(mpi_size);
    const unsigned long zlen = (z1 > z0) ? (z1 - z0) : 0UL;

    double       local_sum_abs_df = 0.0;
    double       local_max_abs_df = 0.0;
    double       local_sum_abs_hf = 0.0;
    double       local_max_abs_hf = 0.0;
    unsigned long local_pixels = 0UL;

    char proc[MPI_MAX_PROCESSOR_NAME];
    int  plen = 0;
    MPI_Get_processor_name(proc, &plen);

    if (zlen == 0UL)
    {
      std::cout << "rank " << rank << '/' << mpi_size << " on " << std::string(proc, static_cast<size_t>(plen))
                << " split_axis=" << axis << " range=[empty] pixels=0\n";
    }
    else
    {
      FloatImageType::RegionType subRegion = inputRegion;
      FloatImageType::IndexType  subIndex = subRegion.GetIndex();
      FloatImageType::SizeType   subSize = inputSize;
      subIndex[axis] += static_cast<itk::IndexValueType>(z0);
      subSize[axis] = static_cast<itk::SizeValueType>(zlen);
      subRegion.SetIndex(subIndex);
      subRegion.SetSize(subSize);

      using ExtractType = itk::ExtractImageFilter<FloatImageType, FloatImageType>;
      auto extract = ExtractType::New();
      extract->SetInput(volume);
      extract->SetExtractionRegion(subRegion);
      extract->Update();
      FloatImageType::Pointer roi = extract->GetOutput();

      // 子块某一维仍可能 <4（例如沿长轴切得很碎或原始层厚极小）；逐块 pad。
      {
        auto                     rsz = roi->GetLargestPossibleRegion().GetSize();
        FloatImageType::SizeType lowerPad;
        FloatImageType::SizeType upperPad;
        constexpr itk::SizeValueType kMinAxis = 4;
        lowerPad.Fill(0);
        upperPad.Fill(0);
        bool needRoiPad = false;
        for (unsigned int d = 0; d < Dimension; ++d)
        {
          if (rsz[d] < kMinAxis)
          {
            upperPad[d] = kMinAxis - rsz[d];
            needRoiPad = true;
          }
        }
        if (needRoiPad)
        {
          using PadRoi = itk::ConstantPadImageFilter<FloatImageType, FloatImageType>;
          auto proi = PadRoi::New();
          proi->SetInput(roi);
          proi->SetPadLowerBound(lowerPad);
          proi->SetPadUpperBound(upperPad);
          proi->SetConstant(0.0f);
          proi->Update();
          roi = proi->GetOutput();
        }
      }

      using CastToDouble = itk::CastImageFilter<FloatImageType, DoubleImageType>;
      auto castUp = CastToDouble::New();
      castUp->SetInput(roi);

      using SmoothD = itk::SmoothingRecursiveGaussianImageFilter<DoubleImageType, DoubleImageType>;
      auto smoothD = SmoothD::New();
      smoothD->SetInput(castUp->GetOutput());
      smoothD->SetSigma(sigma);

      using CastToFloat = itk::CastImageFilter<DoubleImageType, FloatImageType>;
      auto castDown = CastToFloat::New();
      castDown->SetInput(smoothD->GetOutput());

      using SmoothF = itk::SmoothingRecursiveGaussianImageFilter<FloatImageType, FloatImageType>;
      auto smoothF = SmoothF::New();
      smoothF->SetInput(roi);
      smoothF->SetSigma(sigma);

      smoothD->Update();
      smoothF->Update();
      castDown->Update();

      DoubleImageType::Pointer doubleSmoothOut = smoothD->GetOutput();
      FloatImageType::Pointer  floatOnlyOut = smoothF->GetOutput();
      FloatImageType::Pointer  hybridOut = castDown->GetOutput();

      itk::ImageRegionConstIterator<DoubleImageType> itD(doubleSmoothOut, doubleSmoothOut->GetBufferedRegion());
      itk::ImageRegionConstIterator<FloatImageType> itFf(floatOnlyOut, floatOnlyOut->GetBufferedRegion());
      itk::ImageRegionConstIterator<FloatImageType> itH(hybridOut, hybridOut->GetBufferedRegion());

      for (itD.GoToBegin(), itFf.GoToBegin(), itH.GoToBegin(); !itD.IsAtEnd(); ++itD, ++itFf, ++itH)
      {
        const double vd = itD.Get();
        const double vf = static_cast<double>(itFf.Get());
        const double vh = static_cast<double>(itH.Get());
        const double adf = std::fabs(vd - vf);
        const double adh = std::fabs(vd - vh);
        local_sum_abs_df += adf;
        local_sum_abs_hf += adh;
        if (adf > local_max_abs_df)
        {
          local_max_abs_df = adf;
        }
        if (adh > local_max_abs_hf)
        {
          local_max_abs_hf = adh;
        }
        ++local_pixels;
      }

      std::cout << "rank " << rank << '/' << mpi_size << " on " << std::string(proc, static_cast<size_t>(plen))
                << " split_axis=" << axis << " range=[" << z0 << ',' << z1 << ") pixels=" << local_pixels
                << " local_max|D-F|=" << local_max_abs_df << " local_max|D-H|=" << local_max_abs_hf << '\n';
    }

    double global_sum_abs_df = 0.0;
    MPI_Reduce(&local_sum_abs_df, &global_sum_abs_df, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double global_max_abs_df = 0.0;
    MPI_Reduce(&local_max_abs_df, &global_max_abs_df, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    double global_sum_abs_hf = 0.0;
    MPI_Reduce(&local_sum_abs_hf, &global_sum_abs_hf, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double global_max_abs_hf = 0.0;
    MPI_Reduce(&local_max_abs_hf, &global_max_abs_hf, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    unsigned long global_pixels = 0UL;
    MPI_Reduce(&local_pixels, &global_pixels, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
      const double mae_df =
        global_pixels > 0UL ? global_sum_abs_df / static_cast<double>(global_pixels) : 0.0;
      const double mae_hf =
        global_pixels > 0UL ? global_sum_abs_hf / static_cast<double>(global_pixels) : 0.0;
      std::cout << "=== MPI+混合精度汇总 rank0 ===\n";
      std::cout << "file=" << filename << " sigma=" << sigma << " mpi_size=" << mpi_size << '\n';
      std::cout << "pixels=" << global_pixels << '\n';
      std::cout << "[D vs F] doubleGaussian vs floatGaussian: global_max_abs=" << global_max_abs_df
                << " global_mean_abs=" << mae_df << '\n';
      std::cout << "[D vs H] doubleGaussian vs hybrid_final_float: global_max_abs=" << global_max_abs_hf
                << " global_mean_abs=" << mae_hf << '\n';
    }
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cerr << "rank " << rank << " ITK exception: " << err << '\n';
    MPI_Abort(MPI_COMM_WORLD, 2);
  }

  MPI_Finalize();
  return 0;
}
