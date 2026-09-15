# C++ & Edge Vision Deployment Learning

面向**边缘视觉部署**岗位的系统学习仓库：用 C++ 在嵌入式 / 边缘设备上落地视觉算法与模型推理，覆盖从图像处理到部署优化的完整链路。

## 学习定位
| 阶段 | 内容 | 定位 |
|---|---|---|
| 基础 | C++（Primer 精读 + 练习）、Linux 基础 |
| 图像处理 | OpenCV 应用（预处理 / 检测 / 特征提取） | 
| 模型部署 | 模型转换（PyTorch → ONNX）、推理框架（ONNX Runtime / TensorRT / NCNN）、量化与加速 |
| 边缘设备 | Jetson / 瑞芯微 RK 系列等，交叉编译与性能调优 |

## 目录结构

```
c++/
├── exercises/
│   ├── primer/          # C++ Primer 章节练习（ch1~ch8）
│   ├── vision/          # OpenCV 视觉应用练习
│   │   ├── 1.Mat ~ 6.erosionDilation   # 图像基础 / 直方图 / 形态学
│   │   ├── 7.otsuMethod       # Otsu 自动阈值（手写 + LUT + 验证）
│   │   ├── 8.filter           # 空间域滤波
│   │   ├── 9.dft              # 频域滤波（频谱图）
│   │   ├── 10.connected       # 连通域分析
│   │   ├── 11.sobelEdgeDetection  # Sobel 边缘检测
│   │   ├── 12.canny           # Canny + 实时摄像头
│   │   └── 13.matchTemplate   # 模板匹配
│   └── own/             # 自主项目
│       └── myVec/       # 手写 vector 容器（模板版：内存 / 拷贝 / 移动 / 运算符）
├── notes/               # Obsidian 学习笔记（Markdown 双链）
│   └── c++learning/
│       ├── c++Notes/    # C++ 语言笔记
│       ├── visionNotes/ # 视觉笔记
│       └── leetcodeNotes/  # 刷题记录
├── .vscode/             # 编译配置
└── .gitignore
```

## 技术栈与环境

- 语言：C++（C++17）、Python
- 视觉库：OpenCV 4.x
- 部署方向：ONNX Runtime / TensorRT / NCNN（后续阶段）
- 系统：Linux（Ubuntu）+ 嵌入式 Linux
- 开发环境：Windows + VS2022（当前）、Ubuntu VM（OpenCV 阶段）

## 学习资料

- 《C++ Primer（第5版）》
- 《数字图像处理（冈萨雷斯 第3版）》
- OpenCV 官方文档
- ONNX / TensorRT / NCNN 官方文档（后续阶段）

## 当前进度与计划

- [x] C++ 基础：顺序容器、类、函数、IO、泛型算法、RAII
- [x] 自主练习：手写 myVec（动态内存 / 深拷贝 / 移动语义 / 模板化）
- [x] OpenCV 应用：图像读写、灰度、HSV、gamma、直方图、腐蚀膨胀、Otsu 阈值
- [x] OpenCV 滤波：空间域（高斯 / 中值 / 双边 / 可分离核）、频域概念（dft 流程）
- [x] OpenCV 检测：连通域、边缘检测（Sobel / Canny）、模板匹配
- [ ] 相机模型与标定基础
- [ ] Linux 与交叉编译基础
- [ ] 模型部署：PyTorch → ONNX → 推理框架（ONNX Runtime / TensorRT / NCNN）
- [ ] 边缘设备上的实时视觉检测项目

## 说明

- 本仓库为个人学习记录，仅供学习交流
- 练习代码均为本人编写，欢迎交流指正
