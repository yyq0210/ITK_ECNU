#!/bin/bash
# 按已测批次复跑：先绑核 taskset 0-95，再未绑核。
# 结果写 /tmp/*_bound.csv 与 /tmp/*_unbound.csv，再拷回仓库 docs/。
set -u
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
export OMP_NUM_THREADS=96
unset OMP_PROC_BIND OMP_PLACES

HERE=$(cd "$(dirname "$0")" && pwd)
for s in \
  run_1024_remeasure.sh \
  run_fill_missing.sh \
  run_construct_1024.sh \
  run_construct2_1024.sh \
  run_construct3_1024.sh \
  run_path_mesh_1024.sh \
  run_levelset_1024.sh \
  run_metric_1024.sh \
  run_remain_1024.sh \
  run_remain_fix.sh \
  run_extra35_1024.sh \
  run_cat3_1024.sh
do
  if [[ -f "$HERE/$s" ]]; then
    echo "===== RUN $s $(date) ====="
    bash "$HERE/$s" || echo "WARN $s failed"
  elif [[ -f "/tmp/$s" ]]; then
    echo "===== RUN /tmp/$s $(date) ====="
    bash "/tmp/$s" || echo "WARN $s failed"
  else
    echo "SKIP missing $s"
  fi
done
echo "===== ALL_MEASURED_DONE $(date) ====="
ls -l /tmp/*_bound.csv /tmp/*_unbound.csv 2>/dev/null | tail -40
