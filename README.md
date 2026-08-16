# C++ & Machine Vision Learning

面向机器视觉应用工程师岗位的系统学习仓库。

## 学习定位

| 阶段 | 内容 | 定位 |
|---|---|---|
| 基础 | C++（Primer 精读 + 练习） | 语言地基 |
| 图像处理 | OpenCV 应用（灰度 / HSV / 直方图 / 阈值 / 形态学） | 视觉核心能力 |
| 工业视觉 | Halcon | 产线实战工具 |
| 上位机 | Qt（界面 / 图像显示 / 交互） | 完整交付能力 |


## 目录结构

```
c++/
├── exercises/
│   ├── primer/          # C++ Primer 章节练习（ch1~ch8）
│   ├── vision/          # OpenCV 视觉应用练习
│   │   ├── 1.Mat ~ 4.gamma      # 图像基础操作
│   │   ├── 5.drawHist           # 直方图
│   │   ├── 6.erosionDilation    # 腐蚀与膨胀
│   │   └── 7.otsuMethod         # Otsu 自动阈值（手写 + LUT）
│   └── own/             # 自主项目
│       └── myVec/       # 手写 vector 容器（内存管理实践）
├── notes/               # Obsidian 学习笔记（Markdown 双链）
│   └── c++learning/
│       ├── c++Notes/    # C++ 语言笔记
│       ├── visionNotes/ # 视觉笔记
│       └── leetcodeNotes/  # 刷题记录
├── .vscode/             # VSCode 编译配置
└── .gitignore
```

## 技术栈与环境

- 语言：C++（C++11/14/17）
- 视觉库：OpenCV 4.x（当前阶段）
- 后续路线：Halcon、Qt
- 编译：g++ + pkg-config（VSCode tasks 已配置 OpenCV 一键编译）

## 学习资料

- 《C++ Primer（第5版）》
- 《数字图像处理（冈萨雷斯 第3版）》
- OpenCV / Halcon 官方文档
- LeetCode 日常刷题

## 当前进度与计划

- [x] C++ 基础：顺序容器、类、函数、IO
- [x] OpenCV 应用：图像读写、灰度、HSV、gamma、直方图、腐蚀膨胀、Otsu 阈值
- [x] 自主练习：手写 myVec（动态内存 / 深拷贝）
- [ ] OpenCV 进阶：滤波、边缘检测、轮廓、特征匹配
- [ ] Halcon 入门与工业案例
- [ ] Qt 上位机：图像显示、参数界面、项目集成
- [ ] 综合项目：一个完整的视觉检测 Demo

## 说明

- 本仓库为个人学习记录，仅供学习交流
- 练习代码均为本人编写，欢迎交流指正
