#!/bin/bash
# deploy_to_board.sh —— 在 WSL 里跑：打包 → 传到板子 → 配置 → 编译
#
# 用法：
#   ./scripts/deploy_to_board.sh
#   BOARD=root@192.168.1.50 ./scripts/deploy_to_board.sh     # 换板子
#
# 为什么需要这个脚本（两个非标准的地方）：
#   1) 板子上没有 opencv 的 CMake 配置文件（Ubuntu 的 libopencv-*-dev 不带），
#      所以必须 -DOpenCV_DIR 指向项目自带的 cmake/OpenCVConfig.cmake
#   2) detect_stage.cpp 用 rknn_*，必须链接 /usr/lib/librknnrt.so
#
# 每次都会 rm -rf build，因为 CMakeCache.txt 会记住 OpenCV_DIR-NOTFOUND，
# 不清掉的话新参数不生效。

set -e

BOARD=${BOARD:-root@192.168.137.100}
REMOTE_DIR=${REMOTE_DIR:-/root/rk/cpp_pipeline}
MODEL_SRC=${MODEL_SRC:-/root/rk/yolov8n_int8.rknn}

LOCAL_DIR=$(cd "$(dirname "$0")/.." && pwd)
TGZ=/tmp/cpp_pipeline_deploy.tgz

echo "== 0) 检查 =="
echo "  本地: $LOCAL_DIR"
echo "  板子: $BOARD:$REMOTE_DIR"
if [ ! -f "$LOCAL_DIR/cmake/OpenCVConfig.cmake" ]; then
  echo "!! 缺少 cmake/OpenCVConfig.cmake —— 板子上会找不到 OpenCV"
  exit 1
fi
if ! grep -q RKNNRT_LIB "$LOCAL_DIR/src/CMakeLists.txt"; then
  echo "!! src/CMakeLists.txt 里没有 RKNNRT_LIB —— 会链接失败"
  exit 1
fi

echo "== 1) 打包（排除 build / mp4 / bak）=="
tar czf "$TGZ" -C "$LOCAL_DIR" \
  --exclude=build --exclude='*.mp4' --exclude='*.avi' --exclude='*.bak' .
ls -lh "$TGZ"

echo "== 2) 传输 =="
scp -q "$TGZ" "$BOARD:/tmp/"

echo "== 3) 板上解包 / 配置 / 编译 =="
ssh "$BOARD" 'bash -s' <<REMOTE
set -e
mkdir -p "$REMOTE_DIR"
tar xzf /tmp/$(basename "$TGZ") -C "$REMOTE_DIR"
rm -f /tmp/$(basename "$TGZ")
cd "$REMOTE_DIR"

# 模型放好（tests 要读它）
[ -f yolov8n_int8.rknn ] || cp "$MODEL_SRC" . 2>/dev/null || echo "(提示: 模型不在 $MODEL_SRC)"

rm -rf build
cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DOpenCV_DIR=\$(pwd)/cmake

echo "--- 检查 OpenCV_DIR 是否生效 ---"
grep "^OpenCV_DIR:" build/CMakeCache.txt

cmake --build build -j\$(nproc)

echo "--- 产物 ---"
ls -lh build/tests/tests build/tests/benchmark
REMOTE

echo ""
echo "== 完成 =="
echo "在板子上跑："
echo "  ssh $BOARD"
echo "  cd $REMOTE_DIR"
echo "  ./build/tests/benchmark          # CPU 管线基准（不需要摄像头）"
echo "  ./build/tests/tests 15           # 摄像头 + 检测，跑 15 秒"
