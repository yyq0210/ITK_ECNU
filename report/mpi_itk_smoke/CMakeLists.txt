cmake_minimum_required(VERSION 3.16)
project(mpi_itk_smoke LANGUAGES CXX)

find_package(MPI REQUIRED COMPONENTS CXX)
find_package(ITK REQUIRED COMPONENTS ITKSmoothing)
include(${ITK_USE_FILE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mpi_itk_smoke mpi_itk_smoke.cxx)
target_link_libraries(mpi_itk_smoke PRIVATE MPI::MPI_CXX ${ITK_LIBRARIES})

# 第二台节点往往仅有系统旧版 libstdc++；静态嵌入 GCC C++ 运行时，便于无共享库路径时多机运行。
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(mpi_itk_smoke PRIVATE -static-libgcc -static-libstdc++)
endif()
