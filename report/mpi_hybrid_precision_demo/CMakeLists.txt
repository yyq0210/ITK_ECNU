cmake_minimum_required(VERSION 3.16)
project(mpi_hybrid_precision_demo LANGUAGES CXX)

find_package(MPI REQUIRED COMPONENTS CXX)
find_package(
  ITK REQUIRED COMPONENTS ITKSmoothing ITKImageIntensity ITKImageGrid ITKIOImageBase ITKIOMeta ITKIOPNG
)
include(${ITK_USE_FILE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mpi_hybrid_precision_demo mpi_hybrid_precision_demo.cxx)
target_link_libraries(mpi_hybrid_precision_demo PRIVATE MPI::MPI_CXX ${ITK_LIBRARIES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(mpi_hybrid_precision_demo PRIVATE -static-libgcc -static-libstdc++)
endif()
