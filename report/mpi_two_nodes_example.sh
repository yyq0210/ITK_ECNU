#!/usr/bin/env bash
# 在第一台（202.120.87.20 / hpc01）上发起 Open MPI，跨两台机运行。
#
# 要点：
# - 无共享存储时，可执行文件必须在各节点上路径一致（已用 scp 同步 hello_mpi 验证）。
# - root 运行 mpirun 需加 --allow-run-as-root（建议生产环境用普通用户）。
# - 使用 --map-by node 让进程按节点铺开；默认 filling 会先占满第一台。
set -euo pipefail

HOSTFILE="${HOSTFILE:-/home/pub/yyq_archive/mpi_hosts_2nodes.txt}"
NP="${NP:-8}"
EXE="${EXE:-/home/pub/yyq_archive/hello_mpi}"

mpirun --allow-run-as-root \
  --hostfile "${HOSTFILE}" \
  --map-by node \
  -np "${NP}" \
  "${EXE}"
