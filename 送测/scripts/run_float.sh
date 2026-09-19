#!/bin/bash
# 兼容入口：第二个脚本已改为混合精度（float+double+误差），不再只跑单精度。
# 请改用 run_mixed.sh 或 run_test.sh。
set -u
exec bash "$(cd "$(dirname "$0")" && pwd)/run_mixed.sh"
