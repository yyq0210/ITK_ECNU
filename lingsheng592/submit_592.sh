#!/bin/bash
# Submit the qualified-only test on 592 compute cores.
# Skip cn22976 (NFS I/O errors) and cn23018 (dattach timeout).
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
logdir="$HERE/results"
mkdir -p "$logdir"
dsub -q q_hpcapp \
  -n itk-qual592 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 21600 \
  -oo "$logdir/submit.out" \
  -eo "$logdir/submit.err" \
  bash "$HERE/run_qualified.sh"
echo "SUBMITTED logdir=$logdir"
