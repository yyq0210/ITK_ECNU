#!/bin/bash
# 在鲲鹏上编译全部已测 bench。必须 env -i，否则系统 as 2.27 不吃 -march=native。
set -euo pipefail
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin
unset CXXFLAGS CFLAGS CPATH
DEMO=/home/pub/yyq/itk_hybrid_precision_demo
ITK=/home/pub/yyq/ITK-5.4.0/build-yyq
cd "$DEMO"
env -i HOME="$HOME" PATH="$PATH" \
  cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=/home/pub/gjj/gcc10/bin/gcc \
    -DCMAKE_CXX_COMPILER=/home/pub/gjj/gcc10/bin/g++ \
    -DCMAKE_CXX_FLAGS='-O3 -march=armv8.2-a+crypto' \
    -DITK_DIR="$ITK"
cmake --build build -j8 \
  --target precision_pipeline_bench \
  --target precision_bilateral_bench \
  --target precision_conv_bench \
  --target precision_registration_bench \
  --target precision_diffusion_bench \
  --target precision_modules_sweep_bench \
  --target precision_fill_missing_bench \
  --target precision_construct_bench \
  --target precision_path_mesh_bench \
  --target precision_levelset_bench \
  --target precision_metric_bench \
  --target precision_remain_bench \
  --target precision_extra35_bench \
  --target precision_cat3_bench
echo BUILD_ALL_OK
