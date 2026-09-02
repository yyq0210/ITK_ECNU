# 华为鲲鹏 ARM64 推荐编译选项（ITK 5.4 鲲鹏优化版）
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=armv8.2-a+crypto" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=armv8.2-a+crypto" CACHE STRING "" FORCE)

# 避免 -march=native 在旧 binutils 上展开 fp16fml 导致汇编失败
set(ITK_USE_FLOAT_SPACE_PRECISION OFF CACHE BOOL "Optional: float space metadata")
