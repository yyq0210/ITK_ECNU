#!/bin/bash
# Submit the four leftover mixed-precision retries on 592 compute cores.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
dsub -q q_hpcapp \
  -n itk-four592 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 7200 \
  -oo "$logdir/four_retry.out" \
  -eo "$logdir/four_retry.err" \
  bash "$HERE/run_four_retry.sh"
echo "SUBMITTED four-retry logdir=$logdir"
