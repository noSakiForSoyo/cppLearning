# C++ 边缘视觉部署实践

> 在 RK3566（4×Cortex-A55 / 1 TOPS NPU）上用**纯 C++** 部署 YOLOv8n INT8：
> V4L2 采集 → letterbox → RKNN 推理 → 自写 NMS / 坐标反变换 → GStreamer 硬件编码。
>
> **端到端 102.6 ms / 9.6 FPS，连续 30 分钟长稳，峰值 67.5 °C。**

## 核心项目

### 一、RK3566 实时检测部署

| 指标 | 值 |
|---|---|
| 端到端延迟 | **102.63 ms** |
| 持续吞吐 | **9.61 FPS**（1800 s / 17,294 帧） |
| 峰值温度 | **67.5 °C**，全程 1.8 GHz 未降频 |
| 稳定性 | 30 分钟 USB 无掉线、内存无上涨 |
| 输出 | 1280×720 H.264 硬编 |

做过的事，每一条都有数字：

- **INT8 混量级**：`output0` (1,84,8400) 里坐标 0–640 与置信度 0–1 共用一个 per-tensor scale（3.7956），0.75 的分数被量化抹成 0；在 ONNX 末尾插 `Split` 拆成 coords / scores，分数 scale 细化至 0.00294（细了 1290 倍）。
- **整数域后处理**：`want_float=0` + int8 阈值判定，只对过阈框反量化；`decode` 12.27 → 4.95 ms。
- **NMS 阈值**：重复框不是量化问题（float 同样出现，实测 IoU 0.57–0.68），`iou_thres` 0.7 → 0.5 后消除。
- **DDR 调频**：默认 780 MHz 是带宽瓶颈，切 performance（1056 MHz）后单图 180.96 → 126.26 ms；单独钉 NPU 频率无收益，据此排除算力因素。
- **采集帧率匹配消费速率**：30 → 10 fps，丢帧 69% → 3.8%，峰值温度 76.9 → 67.5 °C。

代码：[projects/rknn/cpp/board](projects/rknn/cpp/board) —— 单图 / 视频 / 摄像头(float) / 摄像头(int8) 四个入口。

### 二、多线程图像流水线（可复用组件）

[projects/rknn/cpp/cpp_pipeline](projects/rknn/cpp/cpp_pipeline) —— Source / Stage / Sink 三段接口 + 有界队列。

- 采集线程与消费线程重叠；帧以 `std::move` 入队，线程间不共享可变图像；队列满丢最旧，延迟有界。
- 合成负载 300 帧：单线程 **390.90 FPS** → 流水线 **547.22 FPS**（1.37–1.40x）
- 交叉编译到板上 A55 同负载：**57.01 → 81.10 FPS（1.42x）**
- 作为骨架接入 NPU 检测：端到端 **102.63 ms / 9.61 FPS**

## 仓库结构

```
cppLearning/
├── projects/
│   ├── rknn/
│   │   ├── cpp/board/          # 板端四个检测入口 + 自写后处理
│   │   ├── cpp/cpp_pipeline/   # 多线程流水线 + DetectStage
│   │   ├── cpp/official/       # RKNN 官方参考实现（对照用）
│   │   ├── cpp/rknn_lib/       # rknn_api.h + librknnrt
│   │   ├── quantize.py         # INT8 量化 / 校准
│   │   ├── split_onnx.py       # 输出拆分
│   │   └── compare_int8.py     # 仿真 vs 真机精度对比
│   ├── py-lab/                 # PyTorch 推理 / ONNX 导出 / 手写 NMS / letterbox
│   ├── 2.my_vec/               # 手写 vector + aarch64 交叉编译工具链
│   ├── 4.virtual_demo/         # 多态图像处理管线（filter 抽象）
│   └── ...
├── exercises/                  # C++ Primer 练习 / OpenCV 练习 / 手写容器
├── docs/                       # 4 篇技术文档
└── notes/
```

## 快速开始

```bash
# 普通工程
cd projects/2.my_vec
cmake -B build && cmake --build build

# 交叉编译到 aarch64
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-toolchain.cmake

# 板端检测（在板子上原生编译，见 cpp_pipeline/README.md）
cd projects/rknn/cpp/cpp_pipeline
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR=$(pwd)/cmake
cmake --build build -j4
./build/tests/tests 1800
```

## 技术栈

C++17 · OpenCV 4.2 · RKNN-Toolkit2 2.3.2 · YOLOv8n · GStreamer(mpph264enc) · CMake · Docker · aarch64 交叉编译

## 文档

| 文档 | 内容 |
|---|---|
| [K11C-setup-guide.md](docs/K11C-setup-guide.md) | 板子环境：刷机 / 网络 / SSH / 交叉编译 |
| [RKNN-INT8-踩坑记录.md](docs/RKNN-INT8-踩坑记录.md) | 输出拆头、量化抹平、精度对齐 |
| [P4-RKNN接入指南.md](docs/P4-RKNN接入指南.md) | rknn_init / inputs_set / outputs_get 全链路 |
| [P4-板上跑通记录.md](docs/P4-板上跑通记录.md) | 上板过程与 runtime 版本坑 |

## 手写实现（理解原理）

- **myVec**：动态内存 → 深拷贝 → 移动语义 → 模板化（`exercises/own/myVec`、`projects/2.my_vec`）
- **Otsu**：直方图 → 类间方差最大化 → LUT 二值化，与 OpenCV `THRESH_OTSU` 对比一致
- **NMS / IoU / 坐标反变换**：在板端后处理里实际使用

## 说明

个人学习与工程实践记录。手写实现用于理解原理，生产场景以官方库为准。
