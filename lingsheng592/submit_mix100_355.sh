#!/bin/bash
# Submit retest of the 355 mixable100 dual-criteria operators on 592 cores.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
export RUNS="${RUNS:-1}"
dsub -q q_hpcapp \
  -n itk-mix355 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 43200 \
  -oo "$logdir/mix100_355.out" \
  -eo "$logdir/mix100_355.err" \
  bash -lc "RUNS=$RUNS bash '$HERE/run_mix100_355.sh'"
echo "SUBMITTED mix100-355 RUNS=$RUNS logdir=$logdir"
