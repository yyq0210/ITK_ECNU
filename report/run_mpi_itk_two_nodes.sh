#!/usr/bin/env bash
# 在第一台发起：跨两台运行 mpi_itk_smoke（需已在两台同步同一二进制）。
set -euo pipefail

HOSTFILE="${HOSTFILE:-/home/pub/yyq_archive/mpi_hosts_2nodes.txt}"
NP="${NP:-8}"
EXE="${EXE:-/home/pub/yyq/mpi_itk_smoke/build/mpi_itk_smoke}"

mpirun --allow-run-as-root \
  --hostfile "${HOSTFILE}" \
  --map-by node \
  -np "${NP}" \
  "${EXE}"
