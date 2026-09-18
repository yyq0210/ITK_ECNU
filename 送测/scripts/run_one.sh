#!/bin/bash
# 只跑一个 bench、一个函数。例：
#   bash run_one.sh precision_fill_missing_bench MeanImageFilter
#   bash run_one.sh precision_pipeline_bench
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

EXE_NAME="${1:-}"
OP="${2:-}"
if [[ -z "$EXE_NAME" ]]; then
  echo "用法: bash run_one.sh <precision_xxx_bench> [OperatorName]"
  echo "加 BIND=1 绑核。例: BIND=1 bash run_one.sh precision_fill_missing_bench MeanImageFilter"
  exit 1
fi

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
