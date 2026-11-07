# C++ 多线程图像流水线

一个用 C++17 + OpenCV 写的可复用图像处理流水线。
它把“采集 / 处理 / 输出”拆成 Source / Stage / Sink，并用有界队列 + 一个采集线程把读取和处理重叠起来。

## 架构

    VideoFileSource
          |
          v
    BoundedQueue<Frame>   容量 3 或 64
          |
          v
    Pipeline 消费线程：Stage -> Sink
          |
          v
    ResizeStage -> VideoSink

采集线程只负责 Source::read，把 Frame 推进队列。
消费线程从队列取帧，执行 Stage::process，再执行 Sink::write。
队列满时丢最旧帧，避免延迟越积越大。

## 接口

    struct Frame {
        cv::Mat image;      // BGR
        uint64_t id;
        double timestamp;
    };

    class Source { virtual bool read(Frame&) = 0; };
    class Stage  { virtual bool process(Frame&) = 0; };
    class Sink   { virtual bool write(const Frame&) = 0; };

具体实现：

- VideoFileSource：OpenCV VideoCapture，强制 FFMPEG 后端
- ResizeStage：缩放到 640x360
- VideoSink：OpenCV VideoWriter，使用 mp4v
- NullSink：benchmark 用的空输出

## 构建

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j

benchmark 必须用 Release，否则数字没有意义。

## 运行视频 demo

    ./build/tests/tests /path/to/video.mp4

输出文件为当前目录下的 pipeline_out.mp4。
视频源使用 CAP_FFMPEG，所以文件路径不会被当成 GStreamer 管道解析。

## Benchmark

    ./build/tests/benchmark

合成负载：300 帧、640x480 BGR、9x9 GaussianBlur、NullSink。
WSL 开发机 Release 模式的一次代表性结果：

    single        767.462 ms   390.90 FPS
    pipe cap=64   560.418 ms   535.32 FPS
    pipe cap=3    548.224 ms   547.22 FPS

三种配置都处理完 300 帧，流水线吞吐约提升 1.37–1.40 倍。
cap=3 这一轮没有丢帧，因为生产和消费速度接近；
它是队列容量测试，不是丢帧测试。

数字每次会有几个百分点波动，写简历前建议多跑几次。

## 设计说明

- Frame 通过 std::move 进入队列，线程之间不共享可变图像；
- 消费线程独占当前帧，负责 Stage 和 Sink；
- stop 会唤醒消费者，消费者把队列里剩下的帧处理完再退出；
- 容量 64 用于无损验证，容量 3 用于实时模式；
- benchmark 用 NullSink 排除磁盘 I/O，视频 demo 才是完整路径。

## 已知限制

- 只有一个 worker 线程，不是线程池；
- 还没有 CameraSource；
- 还没有丢帧计数和分阶段耗时；
- 下一步：摄像头源、dropped 计数、每段耗时统计。

## 目录

    include/pipeline/   接口、有界队列、Pipeline 声明
    src/                Pipeline 实现和视频适配器
    tests/              视频 demo 和合成 benchmark
