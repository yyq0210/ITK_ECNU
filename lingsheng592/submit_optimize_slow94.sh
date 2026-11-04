#!/bin/bash
# Retest the 94 err-ok-but-not-faster operators (RUNS=100, one round).
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
export REPEATS="${REPEATS:-1}"
export RUNS="${RUNS:-100}"
export CANDIDATES="${CANDIDATES:-$HERE/optimize_near_miss_candidates.tsv}"
export RESULTS="${RESULTS:-$HERE/results/optimize_slow94_$(date +%Y%m%d_%H%M%S)}"
dsub -q q_hpcapp \
  -n itk-opt94 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 172800 \
  -oo "$logdir/optimize_slow94.out" \
  -eo "$logdir/optimize_slow94.err" \
  bash -lc "REPEATS=$REPEATS RUNS=$RUNS CANDIDATES='$CANDIDATES' RESULTS='$RESULTS' bash '$HERE/run_mixable_100.sh'"
echo "SUBMITTED optimize-slow94 REPEATS=$REPEATS RUNS=$RUNS CANDIDATES=$CANDIDATES"
