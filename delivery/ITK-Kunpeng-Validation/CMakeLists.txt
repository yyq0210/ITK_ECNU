cmake_minimum_required(VERSION 3.16)
project(ITK_Kunpeng_Validation LANGUAGES CXX)

find_package(ITK REQUIRED COMPONENTS
  ITKSmoothing
  ITKImageIntensity
  ITKThresholding
  ITKImageFeature
  ITKConvolution
  ITKRegistrationMethodsv4
  ITKAnisotropicSmoothing
  ITKIOImageBase
  ITKIOPNG
  ITKIOMeta
)
include(${ITK_USE_FILE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BENCHES
  precision_pipeline_bench
  precision_bilateral_bench
  precision_conv_bench
  precision_registration_bench
  precision_diffusion_bench
)

foreach(target IN LISTS BENCHES)
  add_executable(${target} ${target}.cxx)
  target_link_libraries(${target} PRIVATE ${ITK_LIBRARIES})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++)
  endif()
endforeach()
