# 交叉编译到 RK3566（aarch64）
#
# 依赖 sysroot/：由 scripts/make_sysroot.sh 从板子上抽出来
#   usr/include/opencv4
#   usr/lib/aarch64-linux-gnu/libopencv_*.so*
#   usr/lib/librknnrt.so
#
# libc / libstdc++ 用交叉工具链自带的（rk3566-builder 是 Ubuntu 20.04，
# glibc 2.31，和板子一致），所以这里不用 --sysroot，只把第三方库加进来。

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(RK_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/../sysroot" CACHE PATH "board sysroot")

set(CMAKE_CXX_FLAGS_INIT "-I${RK_SYSROOT}/usr/include/opencv4")
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-L${RK_SYSROOT}/usr/lib/aarch64-linux-gnu -L${RK_SYSROOT}/usr/lib -Wl,--allow-shlib-undefined")
