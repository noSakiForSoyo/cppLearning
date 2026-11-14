# cpp_pipeline —— RK3566 上的多线程 YOLOv8n 检测流水线

把「摄像头采集 → letterbox → NPU 推理 → 后处理 → 画框 → H.264 硬编」拆成
Source / Stage / Sink 三段，用**有界队列 + 一个采集线程**把「等摄像头」和「NPU 推理」重叠起来。

纯 C++17 · OpenCV 4.2 · RKNN Runtime 2.5.0 · 目标板 KICKPI K11C（RK3566 / 4×Cortex-A55）

## 架构

```
        采集线程                          主线程（Pipeline::run）
┌──────────────────────┐        ┌─────────────────────────────────────┐
│ CameraSource::read   │        │ DetectStage::process                │
│   cap.read   ~100 ms │        │   letterbox       6.3 ms            │
│   clone              │        │   rknn_run       82.6 ms            │
│   push ──────────────┼──► Q ──┼─► decode_int8     5.0 ms            │
└──────────────────────┘   3    │   反变换 + 画框    0.6 ms            │
                        满则丢最旧 └──────────────┬──────────────────────┘
                                                  ▼
                                        VideoSink（GStreamer mpph264enc）
```

吞吐 = 1 / max(生产者, 消费者)。采集 100 ms、消费 102.6 ms → 9.6 FPS。

## 实测（1280×720，默认 DMC，采集 10 fps）

| 指标 | 值 |
|---|---|
| 端到端（stage） | **102.63 ms** |
| 持续吞吐 | **9.61 FPS**（1800 s / 17,294 帧） |
| 丢帧 | 3.8% |
| 峰值温度 | **67.5°C**，全程 1.8 GHz 未降频 |
| USB 掉线 | 0 次 |
| 内存 | 423 → 479 → 422 MB |
| 输出 | 1280×720 H.264，10 fps，28:49 |

分段（ms/帧）：`run 82.6 / get 7.35 / letter 6.27 / decode 4.97 / input 0.81 / draw 0.63`

> 上一版配置（采集 30 fps）是 106.00 ms / 9.30 FPS、峰值 76.9°C、丢帧 69%。
> 把采集帧率降到与消费帧率匹配后：**丢帧 -65 pp、温度 -9.4°C、吞吐 +3.3%**。

## 关键设计

**1. 线程归属写死**
`VideoCapture` 只在采集线程，`rknn_context` / `VideoWriter` 只在主线程。共享的只有队列。

**2. 有界队列，满则丢最旧**
容量 3。丢最旧而不是丢最新，队列里永远是最新的 3 帧，延迟有界（3 × 102 ms）。

**3. Frame 用 move 过线程边界**
生产者 push 完就不再碰 `cv::Mat`；`cap.read` 的 buffer 必须 clone 才能交出去。

**4. 停止要 drain**
`stop()` 只置标志 + 唤醒；消费者把队列里剩下的帧处理完再退出。

**5. 采集帧率必须匹配消费帧率**
这是本项目最值钱的一条：30 fps 采集下，生产者每秒解 30 帧却只留下 10 帧——
**2/3 的 MJPEG 解码是白干的**，全部变成热。降到 10 fps 后 CPU 从 1.1 核掉到 0.3 核。

## 目录结构

```
cpp_pipeline/
├── include/pipeline/     Source / Stage / Sink 接口、Frame、BoundedQueue、Pipeline
├── include/detect/       DetectStage（RKNN 推理 + 后处理 + 画框）
├── include/video/        CameraSource / VideoFileSource / VideoSink / ResizeStage
├── src/                  实现
├── tests/                tests.cpp（摄像头 demo）、benchmark.cpp（合成负载）
├── cmake/                板端用的最小 OpenCVConfig.cmake
└── scripts/deploy_to_board.sh
```

## 构建

**板端（原生编译）**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR=$(pwd)/cmake
cmake --build build -j4
```

**WSL（本机调试，跑 video demo / benchmark）**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/tests/benchmark
```

## 运行

```bash
./build/tests/tests 1800     # 摄像头实时检测，运行 1800 秒
```

输出 `out_cam_pipeline.mp4`（H.264 硬编）。

## 已知限制

- 只有一个消费线程，不是线程池；NPU 是单核，再加线程也没用。
- `mpph264enc` 只接受**整数**帧率：非整数会写出错误的 VUI，导致容器时长/帧率全错（已踩）。
- 采集帧率写死 10；换配置要同时改 `CameraSource` 和 `VideoSink` 的 fps。
- 因此视频时长比真实时间短约 4%（9.61 帧按 10 写）。

## 相关文档

- `docs/RKNN-INT8-踩坑记录.md` —— 输出拆头 / 量化抹平
- `docs/P4-RKNN接入指南.md` —— rknn_init / inputs_set / outputs_get
- `docs/P4-板上跑通记录.md` —— 上板过程
- `docs/K11C-setup-guide.md` —— 板子环境
