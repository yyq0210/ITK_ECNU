#!/usr/bin/env bash
# FP16 / FMLAL POC：微基准 + ITK Mean 对照
set -euo pipefail

EXP="${EXP:-/home/pub/yyq/fp16_fmlal_experiment}"
DEMO="${DEMO:-/home/pub/yyq/itk_hybrid_precision_demo/build}"
GCC=/home/pub/gjj/gcc10/bin/g++
MARCH_BASE=armv8.2-a+crypto
REPORT="${REPORT:-/tmp/fp16_fmlal_experiment_$(date +%F_%H%M%S).log}"

exec > >(tee "$REPORT") 2>&1

echo "=== FP16 / FMLAL experiment ==="
echo "date=$(date -Iseconds) report=$REPORT"

cd "$EXP"
mkdir -p build

echo
echo "######## 1. 编译 FP16 widen 版 (+fp16) ########"
env -i HOME="$HOME" PATH=/home/pub/gjj/gcc10/bin:/usr/bin:/usr/local/bin \
  "$GCC" -O3 -fopenmp -std=c++17 -march=${MARCH_BASE}+fp16 \
  fp16_mean_microbench.cxx -o build/fp16_mean_microbench_fp16

echo
echo "######## 2. 尝试编译 FMLAL 版 (+fp16fml) ########"
FMLAL_OK=0
if env -i HOME="$HOME" PATH=/home/pub/gjj/gcc10/bin:/usr/bin:/usr/local/bin \
  "$GCC" -O3 -fopenmp -std=c++17 -march=${MARCH_BASE}+fp16fml -DUSE_FP16FML \
  fp16_mean_microbench.cxx -o build/fp16_mean_microbench_fmlal 2>/tmp/fmlal_build.err; then
  FMLAL_OK=1
  echo "FMLAL build: OK"
else
  echo "FMLAL build: FAILED"
  sed -n '1,12p' /tmp/fmlal_build.err
fi

echo
echo "######## 3. 反汇编抽查 (fcvtl / fmlal) ########"
echo "--- fp16 widen binary ---"
objdump -d build/fp16_mean_microbench_fp16 | grep -E 'fcvtl|fmlal|ld1.*h' | head -6 || true
if [[ "$FMLAL_OK" == "1" ]]; then
  echo "--- fmlal binary ---"
  objdump -d build/fp16_mean_microbench_fmlal | grep -i fmlal | head -6 || true
fi

echo
echo "######## 4. 微基准 (1024², radius=15, 96 threads) ########"
taskset -c 0-95 ./build/fp16_mean_microbench_fp16 1024 1024 15 5 96
if [[ "$FMLAL_OK" == "1" ]]; then
  echo
  taskset -c 0-95 ./build/fp16_mean_microbench_fmlal 1024 1024 15 5 96
fi

echo
echo "######## 5. ITK MeanImageFilter 对照 (precision_conv_bench) ########"
if [[ -x "$DEMO/precision_conv_bench" ]]; then
  python3 <<'PY'
from PIL import Image
src = "/home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice256x256.png"
im = Image.open(src)
try:
    resample = Image.Resampling.LANCZOS
except AttributeError:
    resample = Image.LANCZOS
im.resize((1024, 1024), resample).save("/tmp/BrainProtonDensity1024.png")
print("wrote /tmp/BrainProtonDensity1024.png")
PY
  export ITK_GLOBAL_DEFAULT_THREADER=Pool
  export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
  cd "$DEMO"
  taskset -c 0-95 ./precision_conv_bench /tmp/BrainProtonDensity1024.png 3 | grep -E 'Mean|BoxMean|==='
else
  echo "skip: $DEMO/precision_conv_bench not found"
fi

echo
echo "=== done === report=$REPORT fmlal_ok=$FMLAL_OK"
