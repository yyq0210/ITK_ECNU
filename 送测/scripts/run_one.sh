#!/bin/bash
# 只跑一个 bench、一个函数。例：
#   BIND=1 bash run_one.sh precision_fill_missing_bench MeanImageFilter
#   BIND=1 ITK_BENCH_PRECISION=double bash run_one.sh precision_fill_missing_bench ConvolutionImageFilter
# 精度默认 both（float+double）。官方全流程用 run_test.sh。
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
export ITK_BENCH_PRECISION="${ITK_BENCH_PRECISION:-both}"

EXE_NAME="${1:-}"
OP="${2:-}"
if [[ -z "$EXE_NAME" ]]; then
  echo "用法: bash run_one.sh <precision_xxx_bench> [OperatorName]"
  echo "  BIND=1 绑核。ITK_BENCH_PRECISION=double|both|float"
  echo "  例: BIND=1 bash run_one.sh precision_fill_missing_bench MeanImageFilter"
  exit 1
fi
echo "precision=$ITK_BENCH_PRECISION BIND=${BIND:-0} RUNS=$RUNS"

EXE="$BIN/$EXE_NAME"
if [[ ! -x "$EXE" ]]; then
  echo "找不到 $EXE ，先 compile.sh"
  exit 1
fi

WRAP=""
if [[ "${BIND:-0}" == "1" ]]; then
  WRAP="taskset -c $BIND_CPUS"
fi

if [[ "$EXE_NAME" == "precision_registration_bench" ]]; then
  $WRAP "$EXE" "$FIX" "$MOV" 50 "$RUNS"
elif [[ "$EXE_NAME" == "precision_diffusion_bench" ]]; then
  $WRAP "$EXE" "$IMG" 50 0.125 3 "$RUNS"
elif [[ -n "$OP" ]]; then
  $WRAP "$EXE" "$IMG" "$RUNS" "$OP"
else
  $WRAP "$EXE" "$IMG" "$RUNS"
fi
