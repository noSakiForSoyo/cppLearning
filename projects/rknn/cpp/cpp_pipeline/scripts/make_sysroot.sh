#!/bin/bash
# 从板子上抽出交叉编译需要的第三方库（OpenCV 4.2 + librknnrt）
# 用法：./scripts/make_sysroot.sh [user@board]
set -euo pipefail
BOARD="${1:-root@192.168.137.100}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/sysroot"

rm -rf "$DEST"
mkdir -p "$DEST"

ssh -o BatchMode=yes "$BOARD" 'cd / && tar czf - \
  usr/include/opencv4 \
  usr/lib/aarch64-linux-gnu/libopencv_core.so* \
  usr/lib/aarch64-linux-gnu/libopencv_imgproc.so* \
  usr/lib/aarch64-linux-gnu/libopencv_imgcodecs.so* \
  usr/lib/aarch64-linux-gnu/libopencv_videoio.so* \
  usr/lib/librknnrt.so' | tar xzf - -C "$DEST"

echo "sysroot -> $DEST"
du -sh "$DEST"
