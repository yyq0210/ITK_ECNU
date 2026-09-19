#!/bin/bash
# 绑核跑完全部可测函数（Excel 有 I 列的 435 个都能覆盖）。
# 正式入口是 run_port_double.sh / run_mixed.sh / run_test.sh，不要直接调本文件。
# 精度由 ITK_BENCH_PRECISION 决定：
#   double = 只跑双精度（移植）
#   both/mixed = 混合精度（float + double，并算误差）
#   float = 只跑单精度（一般不用）
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

RAW="${ITK_BENCH_PRECISION:-}"
case "$RAW" in
  double|fp64)
    MODE=double
    export ITK_BENCH_PRECISION=double
    ;;
  float|fp32|single)
    MODE=float
    export ITK_BENCH_PRECISION=float
    ;;
  both|mixed)
    MODE=mixed
    export ITK_BENCH_PRECISION=both
    ;;
  *)
    echo "用法: ITK_BENCH_PRECISION=double|both|mixed|float bash $0"
    echo "  double = 移植测试（绑核、全部函数、只跑双精度）"
    echo "  both/mixed = 混合精度（绑核、全部函数、float+double+max_abs）"
    echo "  float = 只跑单精度"
    exit 1
    ;;
esac

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

TAG="${MODE}_bound"
WRAP="taskset -c $BIND_CPUS"

run_suite() {
  local out=$1
  {
    echo "===== PIPELINE $TAG $(date) precision=$MODE ====="
    timeout 900 $WRAP "$BIN/precision_pipeline_bench" "$IMG" "$RUNS"
    echo "===== CONV $TAG $(date) ====="
    timeout 1800 $WRAP "$BIN/precision_conv_bench" "$IMG" "$RUNS"
    echo "===== BILATERAL $TAG $(date) ====="
    timeout 1800 $WRAP "$BIN/precision_bilateral_bench" "$IMG" "$RUNS"
    echo "===== DIFFUSION $TAG $(date) ====="
    timeout 1800 $WRAP "$BIN/precision_diffusion_bench" "$IMG" 50 0.125 3 "$RUNS"
    echo "===== REGISTRATION $TAG $(date) ====="
    timeout 1800 $WRAP "$BIN/precision_registration_bench" "$FIX" "$MOV" 50 "$RUNS"
    echo "===== SUITE_DONE $TAG $(date) ====="
  } > "$out" 2>&1
}

run_list() {
  local exe=$1
  local name=$2
  local out=$3
  local timeout_sec=${4:-300}
  echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$out"
  if [[ ! -x "$exe" ]]; then
    echo "SKIP missing $exe" >&2
    return 0
  fi
  mapfile -t OPS < <("$exe" --list)
  echo "$name ops=${#OPS[@]} precision=$MODE" >&2
  for op in "${OPS[@]}"; do
    [[ -z "$op" ]] && continue
    if skip_serial "$op"; then
      echo "skip serial $op" >&2
      continue
    fi
    echo "== $name $op ==" >&2
    if ! timeout "$timeout_sec" $WRAP "$exe" "$IMG" "$RUNS" "$op" >> "$out" 2>"$OUT/${name}.err"; then
      echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$out"
      tail -6 "$OUT/${name}.err" >&2 || true
    fi
  done
}

echo "===== START $(date) MODE=$MODE BIND=$BIND_CPUS ITK_BENCH_PRECISION=$ITK_BENCH_PRECISION ====="
ls -lh "$IMG" "$FIX" "$MOV"

run_suite "$OUT/suite_${TAG}.txt"
run_list "$BIN/precision_fill_missing_bench" "fill_${TAG}" "$OUT/fill_missing_${TAG}.csv" 300
run_list "$BIN/precision_path_mesh_bench" "path_${TAG}" "$OUT/path_mesh_${TAG}.csv" 180
run_list "$BIN/precision_levelset_bench" "levelset_${TAG}" "$OUT/levelset_${TAG}.csv" 600
run_list "$BIN/precision_metric_bench" "metric_${TAG}" "$OUT/metric_${TAG}.csv" 600
run_list "$BIN/precision_remain_bench" "remain_${TAG}" "$OUT/remain_${TAG}.csv" 300
run_list "$BIN/precision_extra35_bench" "extra35_${TAG}" "$OUT/extra35_${TAG}.csv" 240
run_list "$BIN/precision_cat3_bench" "cat3_${TAG}" "$OUT/cat3_${TAG}.csv" 300

echo "===== ${TAG}_DONE $(date) ====="
ls -l "$OUT"/*"${TAG}"*
echo "FAIL 行数:"
grep -c ',FAIL,' "$OUT"/*"${TAG}"*.csv 2>/dev/null || true
