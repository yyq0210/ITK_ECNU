#!/bin/bash
# Submit retest of the 355 mixable100 dual-criteria operators on 592 cores.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
export RUNS="${RUNS:-1}"
# Optional: RESULTS=/path/to/outdir  -> measurements.csv / summary.txt 写到该目录
# 未设置时 run 脚本默认写到 $HERE/results/mix100_355_<时间戳>/
RESULTS_ARG=""
if [[ -n "${RESULTS:-}" ]]; then
  mkdir -p "$RESULTS"
  RESULTS_ARG="RESULTS='$RESULTS'"
fi
dsub -q q_hpcapp \
  -n itk-mix355 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 43200 \
  -oo "$logdir/mix100_355.out" \
  -eo "$logdir/mix100_355.err" \
  bash -lc "RUNS=$RUNS $RESULTS_ARG bash '$HERE/run_mix100_355.sh'"
echo "SUBMITTED mix100-355 RUNS=$RUNS RESULTS=${RESULTS:-'(default timestamp under results/)'} logdir=$logdir"
