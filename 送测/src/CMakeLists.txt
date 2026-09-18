cmake_minimum_required(VERSION 3.16)
project(ITK_Kunpeng_Validation LANGUAGES CXX)

find_package(ITK REQUIRED COMPONENTS
  ITKSmoothing
  ITKImageIntensity
  ITKThresholding
  ITKImageFeature
  ITKConvolution
  ITKFFT
  ITKImageSources
  ITKRegistrationMethodsv4
  ITKRegistrationCommon
  ITKMetricsv4
  ITKOptimizers
  ITKAnisotropicSmoothing
  ITKImageGradient
  ITKImageGrid
  ITKImageCompare
  ITKImageNoise
  ITKImageStatistics
  ITKImageLabel
  ITKMathematicalMorphology
  ITKBinaryMathematicalMorphology
  ITKDistanceMap
  ITKCurvatureFlow
  ITKAntiAlias
  ITKFastMarching
  ITKDeconvolution
  ITKDenoising
  ITKRegionGrowing
  ITKLabelVoting
  ITKConnectedComponents
  ITKLabelMap
  ITKImageCompose
  ITKDisplacementField
  ITKWatersheds
  ITKClassifiers
  ITKBiasCorrection
  ITKPDEDeformableRegistration
  ITKColormap
  ITKSuperPixel
  ITKPath
  ITKQuadEdgeMesh
  ITKQuadEdgeMeshFiltering
  ITKLevelSets
  ITKImageFusion
  ITKVoronoi
  ITKImageFrequency
  ITKDiffusionTensorImage
  ITKSpatialFunction
  ITKKLMRegionGrowing
  ITKDeformableMesh
  ITKLevelSetsv4
  ITKSpatialObjects
  ITKMesh
  ITKSignedDistanceFunction
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
  precision_modules_sweep_bench
  precision_fill_missing_bench
  precision_path_mesh_bench
  precision_levelset_bench
  precision_metric_bench
  precision_remain_bench
  precision_extra35_bench
  precision_cat3_bench
)

foreach(target IN LISTS BENCHES)
  add_executable(${target} ${target}.cxx)
  target_link_libraries(${target} PRIVATE ${ITK_LIBRARIES})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++)
  endif()
endforeach()

# 与 fill_missing 同一源，历史脚本名 precision_construct_bench
add_executable(precision_construct_bench precision_fill_missing_bench.cxx)
target_link_libraries(precision_construct_bench PRIVATE ${ITK_LIBRARIES})
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(precision_construct_bench PRIVATE -static-libgcc -static-libstdc++)
endif()
