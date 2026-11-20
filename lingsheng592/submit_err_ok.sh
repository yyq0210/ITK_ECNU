#!/bin/bash
# Submit retest of all error-OK mixable operators (max_abs < 1e-5) on 592 cores.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
export RUNS="${RUNS:-1}"
RESULTS_ARG=""
if [[ -n "${RESULTS:-}" ]]; then
  mkdir -p "$RESULTS"
  RESULTS_ARG="RESULTS='$RESULTS'"
fi
dsub -q q_hpcapp \
  -n itk-errok \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 43200 \
  -oo "$logdir/err_ok.out" \
  -eo "$logdir/err_ok.err" \
  bash -lc "RUNS=$RUNS $RESULTS_ARG bash '$HERE/run_err_ok.sh'"
echo "SUBMITTED err-ok RUNS=$RUNS RESULTS=${RESULTS:-'(default timestamp under results/)'} logdir=$logdir"
