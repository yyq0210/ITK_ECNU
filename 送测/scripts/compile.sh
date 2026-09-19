#!/bin/bash
# 编译全部送测 bench。鲲鹏上必须 env -i，避免 kunpeng-simd.sh 注入 -march=native。
# 官方全流程：bash scripts/run_test.sh（会先调本脚本）
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

if [[ ! -d "$ITK_DIR" ]]; then
  echo "ITK_DIR 不存在: $ITK_DIR"
  echo "请设置：export ITK_DIR=/path/to/ITK/build"
  exit 1
fi

echo "ROOT=$ROOT"
echo "ITK_DIR=$ITK_DIR"
echo "CXX=$CXX  CMAKE=$CMAKE"

unset CXXFLAGS CFLAGS CPATH
env -i HOME="${HOME:-/root}" PATH="$PATH" \
  "$CMAKE" -S "$SRC" -B "$BIN" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_CXX_FLAGS='-O3 -march=armv8.2-a+crypto' \
  -DITK_DIR="$ITK_DIR"

"$CMAKE" --build "$BIN" -j"$(nproc 2>/dev/null || echo 8)" \
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

echo BUILD_OK
ls -l "$BIN"/precision_*_bench
