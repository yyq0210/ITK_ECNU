#!/usr/bin/env bash
# 兼容入口：转发到 delivery/prepare_kunpeng_itk_release.sh
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec bash "$ROOT/delivery/prepare_kunpeng_itk_release.sh" "$@"
