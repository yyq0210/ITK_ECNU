#!/usr/bin/env bash
# 第一台发起：MPI + ImageFileReader 体数据 + 混合精度（默认示例 MHA）。
set -euo pipefail

HOSTFILE="${HOSTFILE:-/home/pub/yyq_archive/mpi_hosts_2nodes.txt}"
NP="${NP:-8}"
EXE="${EXE:-/home/pub/yyq/mpi_hybrid_precision_demo/build/mpi_hybrid_precision_demo}"
VOL="${VOL:-/home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensity3Slices.mha}"
SIGMA="${SIGMA:-2}"

mpirun --allow-run-as-root \
  --hostfile "${HOSTFILE}" \
  --map-by node \
  -np "${NP}" \
  "${EXE}" "${VOL}" "${SIGMA}"
