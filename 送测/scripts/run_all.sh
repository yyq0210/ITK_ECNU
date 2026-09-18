#!/bin/bash
# 统一送测：每个函数 未绑核 + 绑核，各 3 次平均。
# 结果写到 ../results/
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

python3 "$ROOT/scripts/prepare_data.py" || true

if [[ ! -x "$BIN/precision_pipeline_bench" ]]; then
  echo "未编译。先执行: bash $ROOT/scripts/compile.sh"
  exit 1
fi

skip_serial() {
  case "$1" in
    *ByReconstruction*|*HMaxima*|*HMinima*|*HConvex*|*HConcave*|*Fillhole*|*GrindPeak*|*RegionalMaxima*|*RegionalMinima*|*BinaryThinning*|*BinaryPruning*|*FastMarching*|*ConnectedThreshold*|*ConfidenceConnected*|*IsolatedConnected*|*NeighborhoodConnected*|*VectorConfidence*)
      return 0 ;;
  esac
  return 1
}

# 整套 bench（一次跑完内部所有算子）
run_suite() {
  local tag=$1
  local wrap=$2
  local out=$3
  {
    echo "===== PIPELINE $tag $(date) ====="
    timeout 900 $wrap "$BIN/precision_pipeline_bench" "$IMG" "$RUNS"
    echo "===== CONV $tag $(date) ====="
    timeout 1800 $wrap "$BIN/precision_conv_bench" "$IMG" "$RUNS"
    echo "===== BILATERAL $tag $(date) ====="
    timeout 1800 $wrap "$BIN/precision_bilateral_bench" "$IMG" "$RUNS"
    echo "===== DIFFUSION $tag $(date) ====="
    timeout 1800 $wrap "$BIN/precision_diffusion_bench" "$IMG" 50 0.125 3 "$RUNS"
    echo "===== REGISTRATION $tag $(date) ====="
    timeout 1800 $wrap "$BIN/precision_registration_bench" "$FIX" "$MOV" 50 "$RUNS"
    echo "===== SUITE_DONE $tag $(date) ====="
  } > "$out" 2>&1
}

# 按 --list 逐个算子（一个崩溃不影响其余）
run_list() {
  local exe=$1
  local tag=$2
  local wrap=$3
  local out=$4
  local timeout_sec=${5:-300}
  echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$out"
  if [[ ! -x "$exe" ]]; then
    echo "SKIP missing $exe" >&2
    return 0
  fi
  mapfile -t OPS < <("$exe" --list)
  echo "$tag ops=${#OPS[@]}" >&2
  for op in "${OPS[@]}"; do
    [[ -z "$op" ]] && continue
    if skip_serial "$op"; then
      echo "skip serial $op" >&2
      continue
    fi
    echo "== $tag $op ==" >&2
    if ! timeout "$timeout_sec" $wrap "$exe" "$IMG" "$RUNS" "$op" >> "$out" 2>"$OUT/${tag}.err"; then
      echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$out"
      tail -6 "$OUT/${tag}.err" >&2 || true
    fi
  done
}

echo "===== START $(date) ROOT=$ROOT nproc=$(nproc 2>/dev/null || echo ?) ====="
ls -lh "$IMG" "$FIX" "$MOV"

echo "===== BOUND taskset $BIND_CPUS ====="
WRAP="taskset -c $BIND_CPUS"
run_suite bound "$WRAP" "$OUT/suite_bound.txt"
run_list "$BIN/precision_fill_missing_bench" fill_bound "$WRAP" "$OUT/fill_missing_bound.csv" 300
run_list "$BIN/precision_path_mesh_bench" path_bound "$WRAP" "$OUT/path_mesh_bound.csv" 180
run_list "$BIN/precision_levelset_bench" levelset_bound "$WRAP" "$OUT/levelset_bound.csv" 600
run_list "$BIN/precision_metric_bench" metric_bound "$WRAP" "$OUT/metric_bound.csv" 600
run_list "$BIN/precision_remain_bench" remain_bound "$WRAP" "$OUT/remain_bound.csv" 300
run_list "$BIN/precision_extra35_bench" extra35_bound "$WRAP" "$OUT/extra35_bound.csv" 240
run_list "$BIN/precision_cat3_bench" cat3_bound "$WRAP" "$OUT/cat3_bound.csv" 300

echo "===== UNBOUND ====="
WRAP=""
run_suite unbound "$WRAP" "$OUT/suite_unbound.txt"
run_list "$BIN/precision_fill_missing_bench" fill_unbound "$WRAP" "$OUT/fill_missing_unbound.csv" 300
run_list "$BIN/precision_path_mesh_bench" path_unbound "$WRAP" "$OUT/path_mesh_unbound.csv" 180
run_list "$BIN/precision_levelset_bench" levelset_unbound "$WRAP" "$OUT/levelset_unbound.csv" 600
run_list "$BIN/precision_metric_bench" metric_unbound "$WRAP" "$OUT/metric_unbound.csv" 600
run_list "$BIN/precision_remain_bench" remain_unbound "$WRAP" "$OUT/remain_unbound.csv" 300
run_list "$BIN/precision_extra35_bench" extra35_unbound "$WRAP" "$OUT/extra35_unbound.csv" 240
run_list "$BIN/precision_cat3_bench" cat3_unbound "$WRAP" "$OUT/cat3_unbound.csv" 300

echo "===== ALL_DONE $(date) ====="
ls -l "$OUT"
