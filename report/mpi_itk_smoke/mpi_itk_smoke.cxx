#include <mpi.h>

#include <cstdint>
#include <cstdio>

#include "itkImage.h"
#include "itkImageRegionConstIterator.h"
#include "itkMedianImageFilter.h"

// 每个 MPI rank 在本地构造小图、跑 ITK Median 滤波，再用 MPI_Reduce 汇总校验多机确实在执行 ITK。
int
main(int argc, char ** argv)
{
  MPI_Init(&argc, &argv);

  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  constexpr unsigned int Dimension = 2;
  using PixelType = unsigned char;
  using ImageType = itk::Image<PixelType, Dimension>;

  auto image = ImageType::New();
  ImageType::RegionType region;
  ImageType::SizeType   sz;
  sz.Fill(64);
  ImageType::IndexType idx;
  idx.Fill(0);
  region.SetIndex(idx);
  region.SetSize(sz);
  image->SetRegions(region);
  image->Allocate(true);
  const auto seed = static_cast<PixelType>((static_cast<unsigned>(rank) * 17U + 5U) & 0xFFU);
  image->FillBuffer(seed);

  using MedianType = itk::MedianImageFilter<ImageType, ImageType>;
  auto median = MedianType::New();
  typename MedianType::InputSizeType radius;
  radius.Fill(1);
  median->SetRadius(radius);
  median->SetInput(image);
  median->Update();

  auto                                             output = median->GetOutput();
  itk::ImageRegionConstIterator<ImageType> it(output, output->GetBufferedRegion());
  std::uint64_t                                    local_sum = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    local_sum += static_cast<std::uint8_t>(it.Get());
  }

  std::uint64_t global_sum = 0;
  MPI_Reduce(&local_sum, &global_sum, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  char proc[MPI_MAX_PROCESSOR_NAME];
  int  plen = 0;
  MPI_Get_processor_name(proc, &plen);

  std::printf(
    "rank %d/%d on %.*s itk_median_local_pixel_sum=%llu\n", rank, size, plen, proc, static_cast<unsigned long long>(local_sum));

  if (rank == 0)
  {
    std::printf("itk_median_global_pixel_sum=%llu mpi_size=%d\n",
                static_cast<unsigned long long>(global_sum),
                size);
  }

  MPI_Finalize();
  return 0;
}
