# 板端用的最小 OpenCVConfig.cmake
#
# 背景：板子上装了 libopencv-{core,imgproc,imgcodecs,videoio}-dev（有头文件和 .so），
# 但 Ubuntu 的这些包里【没有】OpenCV 的 CMake 配置文件，
# 所以 find_package(OpenCV REQUIRED) 会失败。
#
# 用法（在板子上）：
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR=$(pwd)/cmake
#
# 只声明本项目实际用到的四个模块。

set(OpenCV_FOUND TRUE)
set(OpenCV_VERSION "4.2.0")
set(OpenCV_INCLUDE_DIRS "/usr/include/opencv4")
set(OpenCV_LIBS opencv_core opencv_imgproc opencv_imgcodecs opencv_videoio)
