#!/usr/bin/env bash
# 生成鲲鹏 ITK 结项交付包（完整源码 + 可选验证程序 + tarball）
#
# 用法:
#   bash delivery/prepare_kunpeng_itk_release.sh <ITK_SRC> <OUT_DIR> [-t]
#
# 示例（在 202.120.87.86）:
#   bash delivery/prepare_kunpeng_itk_release.sh \
#     /home/pub/yyq/ITK-5.4.0 \
#     /home/pub/yyq/release/ITK-5.4.0-Huawei-Kunpeng \
#     -t
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ITK_SRC="${1:?usage: $0 <ITK_SRC> <OUT_DIR> [-t]}"
OUT_DIR="${2:?usage: $0 <ITK_SRC> <OUT_DIR> [-t]}"
MAKE_TAR="${3:-}"

RELEASE_ROOT="$(dirname "$OUT_DIR")"
VALIDATION_DIR="${RELEASE_ROOT}/ITK-Kunpeng-Validation"

ARGS=("$ITK_SRC" "$OUT_DIR" "--validation-dir" "$VALIDATION_DIR")
if [[ "$MAKE_TAR" == "-t" ]]; then
  ARGS+=(-t)
fi

python3 "$SCRIPT_DIR/apply_kunpeng_to_itk.py" "${ARGS[@]}"

echo ""
echo "Deliver:"
echo "  1) $OUT_DIR"
echo "  2) $VALIDATION_DIR"
if [[ "$MAKE_TAR" == "-t" ]]; then
  echo "  3) ${OUT_DIR}.tar.gz"
fi
