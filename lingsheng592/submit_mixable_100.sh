#!/bin/bash
# Submit one-round mixable retest: every previously-tested mixable op, RUNS=100 each.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
export REPEATS="${REPEATS:-1}"
export RUNS="${RUNS:-100}"
# 100 inner runs ≈ day-scale; allow 72h wallclock.
dsub -q q_hpcapp \
  -n itk-mix100 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 259200 \
  -oo "$logdir/mixable100.out" \
  -eo "$logdir/mixable100.err" \
  bash -lc "REPEATS=$REPEATS RUNS=$RUNS bash '$HERE/run_mixable_100.sh'"
echo "SUBMITTED mixable100 REPEATS=$REPEATS RUNS=$RUNS logdir=$logdir"
