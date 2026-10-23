# C++ & Edge Vision Deployment Learning

面向**边缘视觉部署**岗位的学习与工程实践仓库：以 C++ 为主，覆盖传统图像处理、深度学习模型部署链路（PyTorch → ONNX → RKNN）、交叉编译与边缘设备落地。

## 学习定位

| 阶段 | 内容 | 目标 |
|---|---|---|
| 语言基础 | C++ Primer 精读、泛型算法、RAII、多态与智能指针 | 工程语言能力 |
| 图像处理 | OpenCV：预处理、滤波、阈值分割、边缘检测、模板匹配、相机模型 | 视觉算法基础 |
| 模型链路 | PyTorch / YOLOv8 → ONNX → RKNN，NMS、letterbox、INT8 量化 | 部署前处理 |
| 边缘落地 | aarch64 交叉编译、Docker 构建环境、RK3566 设备部署 | 目标平台交付 |

## 目录结构

```
cppLearning/
├── exercises/                     # 基础练习
│   ├── primer/                    # C++ Primer 章节练习（ch1~ch8）
│   ├── vision/                    # OpenCV 应用练习（1.Mat ~ 13.matchTemplate）
│   └── own/myVec/                 # 手写 vector 容器（模板版）
├── projects/                      # 工程实践（CMake 组织，include/src/app/tests）
│   ├── 1.test_demo/               # CMake 工程结构入门
│   ├── 2.my_vec/                  # myVec 模块化重构 + aarch64 交叉编译工具链
│   ├── 3.cv_demo/                 # OpenCV 集成工程
│   ├── 4.virtual_demo/            # 多态图像处理管线（filter 抽象 + pipe_line）
│   ├── py-lab/                    # Python 实验：PyTorch / ONNX / NMS / letterbox / YOLO
│   ├── rknn/                      # RKNN 部署管线：转换、输出拆分、INT8 量化、精度对比
│   ├── CMakeLists.txt             # 顶层构建
│   ├── create.sh                  # 新项目脚手架
│   ├── py.sh                      # Python 虚拟环境入口
│   └── scp.sh                     # 交叉编译与设备部署
├── project/                       # Windows 侧 VS2022 CMake 工程
├── notes/c++learning/             # Obsidian 笔记（C++ / 视觉 / LeetCode）
└── a.sh
```

## 项目亮点

### 一、手写算法：理解原理而非只会调 API

- **Otsu 自动阈值**（`exercises/vision/7.otsuMethod`）
  直方图统计 → 类间方差最大化 → LUT 查表二值化；手写实现与 OpenCV `THRESH_OTSU` 结果验证一致。
- **myVec 容器**（`exercises/own/myVec`、`projects/2.my_vec`）
  动态内存管理 → 深拷贝（拷贝构造/赋值）→ 移动语义（noexcept）→ 模板化；另有 `unique_ptr` 版本实现。

### 二、多态图像处理管线（`projects/4.virtual_demo`）

- `filter` 抽象基类：虚析构 + 纯虚 `apply()` / `name()` 接口
- `pipe_line` 以 `std::vector<std::unique_ptr<filter>>` 组合处理器，运行期动态组装处理链（灰度、高斯滤波等）
- 练习要点：多态、智能指针所有权、可扩展管线设计

### 三、边缘部署链路（`projects/rknn` + `projects/py-lab`）

- **前处理**：letterbox 缩放、手写 NMS
- **模型导出与探查**：PyTorch → ONNX、ONNX 结构探查
- **RKNN 转换**：`rknn.load_onnx` → `rknn.build`（`target_platform = rk3566`，含 mean/std 归一化配置）
- **INT8 量化**：`quantized_dtype = w8a8` + 校准集（`calib.txt` / `calib_self.txt`）
- **工程处理**：ONNX 输出拆分、仿真验证、INT8 与浮点精度对比

### 四、交叉编译与设备部署（`projects/2.my_vec` + `scp.sh`）

- CMake toolchain：`CMAKE_SYSTEM_PROCESSOR=aarch64`、`aarch64-linux-gnu-g++`
- Docker 构建镜像（rk3566-builder）保证环境可复现
- 构建产物通过 `scp` 部署至边缘设备

## 技术栈与环境

- 语言：C++17、Python 3
- 视觉：OpenCV 4.x
- 深度学习：PyTorch、YOLOv8、ONNX / ONNX Runtime
- 边缘部署：RKNN Toolkit2、RK3566、aarch64 交叉编译
- 工程工具：CMake、Docker、Git
- 开发环境：Windows（VS2022）+ WSL2（Ubuntu）

## 构建方式

`projects/` 下各工程独立组织，源码位于 `include` / `src` / `app` / `tests`：

```bash
cd projects/<project>
cmake -B build && cmake --build build

# 交叉编译（以 my_vec 为例）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-toolchain.cmake
```

## 当前进度

- [x] C++ 基础：顺序容器、类、函数、IO、泛型算法、RAII
- [x] 手写 myVec：内存管理 / 深拷贝 / 移动语义 / 模板化
- [x] OpenCV：图像基础、滤波（空间域/频域）、阈值分割、边缘检测、连通域、模板匹配
- [x] C++ 工程实践：CMake 多模块组织、多态管线、智能指针
- [x] Python 链路：PyTorch 推理、NMS、letterbox、ONNX 导出与探查
- [x] RKNN 转换与 INT8 量化（含校准集与精度对比）
- [x] aarch64 交叉编译与设备部署
- [ ] 相机模型与标定
- [ ] 完整边缘视觉检测项目（检测 → 量化 → 设备端实时推理）

## 学习资料

- 《C++ Primer（第5版）》
- 《数字图像处理（冈萨雷斯 第3版）》
- OpenCV / ONNX / RKNN Toolkit2 官方文档

## 说明

- 本仓库为个人学习与实践记录，仅供学习交流
- 练习代码均为本人编写；手写实现（Otsu、myVec、NMS）用于理解原理，生产场景以官方库为准
