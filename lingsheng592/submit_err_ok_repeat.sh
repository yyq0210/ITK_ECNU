#!/bin/bash
# Submit multi-round retest of all mixable operators with prior max_abs < 1e-5.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
export REPEATS="${REPEATS:-5}"
dsub -q q_hpcapp \
  -n itk-errok5 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 43200 \
  -oo "$logdir/err_ok_repeat.out" \
  -eo "$logdir/err_ok_repeat.err" \
  bash -lc "REPEATS=$REPEATS bash '$HERE/run_err_ok_repeat.sh'"
echo "SUBMITTED err-ok-repeat REPEATS=$REPEATS logdir=$logdir"
