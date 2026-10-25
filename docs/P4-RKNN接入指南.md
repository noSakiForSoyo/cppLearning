# P4：把 RKNN 接进来 —— 完整指南

> 汇总版。从"我们在做什么"到"init_yolov8_model 每一行在干什么"，到"该理解多深"。
> 配套材料：`~/projects/rknn/cpp/official/{main.cc, yolov8.cc, yolov8.h}`

---

# 0. 一句话目标

> **在 RK3566 板子上，用纯 C++ 跑一个实时的 YOLOv8n 目标检测系统，并测出瓶颈、做过优化。**

简历用途：嵌入式视觉 + 模型部署 + 性能优化。

---

# 1. 数据流（**记住这张图就够了**）

```
摄像头 / 视频文件
        ↓
┌────────────────────────────────┐
│ ① 预处理                        │
│   letterbox → 640×640           │
│   BGR → RGB                     │
│   NHWC, uint8                   │
│   ★ 同时记下 scale / pad        │
└───────────────┬────────────────┘
                ↓
┌────────────────────────────────┐
│ ② RKNN 推理（NPU）              │  ← ★ P4 现在要接的
│   rknn_init                     │
│   rknn_inputs_set               │
│   rknn_run                      │
│   rknn_outputs_get              │
└───────────────┬────────────────┘
                ↓
      coords (4, 8400)   scores (80, 8400)
                ↓
┌────────────────────────────────┐
│ ③ 后处理                        │  ← ✅ 已完成（test_nms.cpp）
│   argmax → conf / cls           │
│   conf > 0.25 过滤               │
│   cxcywh → xyxy                 │
│   NMS（按类别）                  │
└───────────────┬────────────────┘
                ↓
┌────────────────────────────────┐
│ ④ 坐标映射回原图                │
│   (x - pad) / scale             │
└───────────────┬────────────────┘
                ↓
┌────────────────────────────────┐
│ ⑤ 画框 / 显示 / 保存            │
└────────────────────────────────┘
```

⚠️ **`letterbox` 埋的坑**：推理在 640×640 上做，用户看的是原图，所以 ④ 必须用 ① 记下的 `scale` / `pad` 反变换。

---

# 2. 进度

| 块 | 状态 |
|---|---|
| 板子环境 / C++ 工程化 / OS+多线程 / Python+模型 | ✅ |
| **P3：模型转换 + INT8 量化** | ✅（含"输出张量拆分"那个关键发现） |
| **③ 后处理（C++）** | ✅（真实数据验证，4 个框逐位对上） |
| **② RKNN 推理** | ⬜ ← **P4 剩下的核心** |
| ① 预处理（C++） | 🟡 Python 里有，C++ 还没写 |
| ④ 坐标反变换 | ⬜ |
| ⑤ 画框 | ⬜ |
| 交叉编译 + 上板 / 视频输入 / baseline 数字 | ⬜ |
| P5：优化 | ⬜ |

---

# 3. `init_yolov8_model` 六步详解

## 3.0 先分清三个"层次"

| 层 | 是什么 | 类比 |
|---|---|---|
| **`rknn_context`** | 不透明句柄，代表"一次模型加载" | `FILE*` |
| **`rknn_tensor_attr`** | 每个张量的元数据（形状/类型/格式/量化参数） | `ndarray.shape` + `dtype` |
| **`rknn_app_context_t`** | 你自己的结构体，把上面两样存起来 | 你写的 class |

---

## 3.1 `rknn_init` —— 加载模型

```c
char *model;
model_len = read_data_from_file(model_path, &model);   // 整个文件读进内存
ret = rknn_init(&ctx, model, model_len, 0, NULL);
free(model);                                           // 立刻释放，安全
```

**参数：**

| 参数 | 含义 |
|---|---|
| `&ctx` | 出参，成功后填上句柄 |
| `model` | **模型数据的指针（不是路径！）** |
| `model_len` | 字节数 |
| `0` | flags，0 = 默认 |
| `NULL` | extend 扩展配置，用不上 |

### ⭐ 关键点 1：C API 要"内容"，不要"路径"

| | Python 版 | C 版 |
|---|---|---|
| 加载 | `rknn.load_rknn('a.rknn')` | **读进内存 → 传指针** |

**为什么**：C API 不假设你有文件系统。板子上模型可能烧在 flash、可能 mmap、可能从网络收到。
**"从哪来"是你的事，"是什么"是它的事。**

### ⭐ 关键点 2：`free(model)` 之后模型还能用

因为 `rknn_init` 内部已经把模型**拷进自己管理的内存**了。
**和 `unique_ptr` 移交所有权是同一个思想。**

---

## 3.2 `rknn_query(IN_OUT_NUM)` —— 问"几个输入输出"

```c
rknn_input_output_num io_num;
ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
```

```c
typedef struct {
    uint32_t n_input;
    uint32_t n_output;
} rknn_input_output_num;
```

我们的模型：`model input num: 1, output num: 2`

### ⭐ `rknn_query` 的调用模式（后面还要用两次）

```
你给：一个空结构体 + sizeof
它做：往结构体里填数据
你查：ret == RKNN_SUCC ?
```

⚠️ `sizeof(...)` 不能省 —— RKNN 靠它判断结构体版本（向前兼容）。

### ⭐ 为什么不能写死"1 进 1 出"

因为我们的模型是 **1 进 2 出**（拆过输出），别人可能是 1 进 1 出。
**写死 = 只对这一个模型有效；问 = 对所有模型有效。** 这就是"让代码适配模型"。

---

## 3.3 建 `rknn_tensor_attr` 数组

```c
rknn_tensor_attr input_attrs[io_num.n_input];   // VLA，大小运行时定
```

### `rknn_tensor_attr` 里装什么

```c
typedef struct _rknn_tensor_attr {
    uint32_t index;                   // 第几个张量（你自己填）
    char     name[RKNN_MAX_NAME_LEN]; // 名字，如 "images"
    uint32_t n_dims;                  // 几维
    uint32_t dims[RKNN_MAX_DIMS];     // 每维多大，如 [1,640,640,3]
    uint32_t n_elems;                 // 总元素数
    uint32_t size;                    // 总字节数
    rknn_tensor_format   fmt;         // NCHW / NHWC
    rknn_tensor_type     type;        // INT8 / UINT8 / FLOAT16 / FLOAT32
    rknn_tensor_qnt_type qnt_type;    // 量化类型
    int8_t zp;                        // ★ zero point
    float  scale;                     // ★ 量化 scale
} rknn_tensor_attr;
```

### ⭐ `zp` / `scale` 是什么

**int8 存的是整数，代表的值是浮点数：**

```
真实值 = (存的值 - zp) × scale
```

**我们模型的例子：**
```
scores 的 scale ≈ 0.00293975
存的值 255 → (255 - zp) × 0.00293975 ≈ 0.7496   ← 那个 cup 的置信度
```

- **`scale` = "每一格代表多少"**（P3 撞到的那个 `3.7956` 就是 scale）
- **`zp` = "数字 0 对应的真实值不是 0 时要减的偏移"**

---

## 3.4 `rknn_query(INPUT_ATTR)` —— 填充数组

```c
for (int i = 0; i < io_num.n_input; i++) {
    input_attrs[i].index = i;                     // ★ 先说"第几个"
    ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR,
                     &(input_attrs[i]),
                     sizeof(rknn_tensor_attr));
    dump_tensor_attr(&(input_attrs[i]));
}
```

### ⭐ 为什么必须先 `index = i`

**API 只认结构体里的 `index` 字段，不认数组下标。**

### 打印出来大概是这样

```
index=0, name=images, n_dims=4, dims=[1, 640, 640, 3], n_elems=1228800,
size=1228800, fmt=NHWC, type=UINT8, qnt_type=AFFINE_ASYMMETRIC, zp=-128, scale=1.000000
```

| 字段 | 对我们的意义 |
|---|---|
| `dims=[1,640,640,3]` | NHWC |
| `type=UINT8` | **所以喂 uint8，不是 float** ⭐ |
| `scale=1.0` | 配合 `std=255`，内部自行除 255 |

> **⭐ 这几行打印，直接回答"我应该喂什么格式" —— 不用猜。**

---

## 3.5 输出：一模一样的套路

```c
rknn_tensor_attr output_attrs[io_num.n_output];
for (int i = 0; i < io_num.n_output; i++) {
    output_attrs[i].index = i;
    rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
    dump_tensor_attr(&(output_attrs[i]));
}
```

会打印：
```
index=0, name=coords, dims=[1,4,8400],  type=INT8, ...
index=1, name=scores, dims=[1,80,8400], type=INT8, ...
```

⚠️ 这是 **RKNN 给的形状**，可能和 ONNX 里不同（reshape / layout 都可能改）。

---

## 3.6 `is_quant` + 存进 `app_ctx`

### 6a. 判断量化

```c
if (output_attrs[0].qnt_type == RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC &&
    output_attrs[0].type == RKNN_TENSOR_INT8)
    is_quant = true;
else
    is_quant = false;
```

**用途**：后面 `want_float = (!is_quant)` —— 量化模型保持 int8 自己反量化（快），浮点模型直接要 float。

### 6b. 为什么要 `malloc` + `memcpy` ⭐

```c
app_ctx->input_attrs = malloc(...);
memcpy(app_ctx->input_attrs, input_attrs, ...);
```

**因为 `input_attrs` 是栈上的局部数组：**

```
init_yolov8_model() 返回
    ↓
input_attrs 这块栈内存没了
    ↓
但 inference_yolov8_model() 还要用里面的 scale / zp / dims
    ↓
所以必须复制到活得更久的地方（堆），由 app_ctx 持有
```

> **这就是 C 的"所有权"问题。**
> **你在 C++ 里用 `unique_ptr` / 成员变量解决它，这里手写 `malloc` + `memcpy`。**
> **对应的 `free()` 在 `release_yolov8_model` 里 —— 就是手写的"析构"。**

### 6c. 确定输入尺寸

```c
if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
    channel = dims[1];  height = dims[2];  width = dims[3];
} else {                                     // NHWC
    height  = dims[1];  width  = dims[2];  channel = dims[3];
}
```

> **⭐ 这就是"为什么不硬编码 640×640×3"。**
> **换 320×320 时，这段代码不用改，`app_ctx->model_width` 自动变 320。**

---

# 4. 该理解到什么深度（A / B / C）

| 层 | 内容 | 要求 |
|---|---|---|
| **A. 必须深挖** | · `scale`/`zp` 与反量化公式<br>· `rknn_query` 的"先问再干"<br>· 输出的 dtype（int8 还是 float）<br>· 输入格式/类型与 `std_values` 的对应 | **能讲"为什么"，能自己写** |
| **B. 知道有、用时查** | · `rknn_init` 参数含义<br>· `rknn_tensor_attr` 每个字段<br>· `RKNN_QUERY_*` 枚举<br>· `flags` 取值 | **不用背**，`grep rknn_api.h` 即可 |
| **C. 纯样板** | · `memset` 清零<br>· `malloc` + `memcpy`<br>· `if (ret != RKNN_SUCC)`<br>· `free` 清理 | **照抄，不深究** |

## ⭐ 判断标准（比上面那张表重要）

> ## **这段代码出错时，会不会报错？**
>
> - **会报错** → 样板，会用就行（报错会告诉你去查）
> - **不报错、只是结果错** → 必须深挖（没人告诉你，你会花三天）

| 情况 | 报错吗 | 该多深 |
|---|---|---|
| 模型路径写错 | ✅ | 浅 |
| `rknn_query` 参数错 | ✅ | 浅 |
| **喂 float32 给 uint8 模型** | ❌ | **深** |
| **`want_float=0` 却按 float 读 buf** | ❌ | **深** |
| **输出张量混量级**（P3） | ❌ | **深** |

> **"会报错"是工程问题，"不报错"才是要学到手的东西 —— 也是简历上能写故事的。**

---

# 5. 三道自测题

### ① 模型换成 320×320，代码要改哪里？
**答：不用改。** `model_width` 是 query 出来的，自动变 320。

### ② `want_float = 0` 时，`outputs[i].buf` 里是什么？
**答：int8 原始整数。** 要自己算 `float v = (int8_val - attrs[i].zp) * attrs[i].scale;`

### ③ 喂 float32 [0,1] 给 `uint8 + std=255` 的模型会怎样？
**答：不报错，但输入变成 [0, 1/255]，几乎全黑，检测不到东西。**
（这就是 10 月 19 号踩的坑。）

---

# 6. 官方代码：抄什么、不抄什么

## ✅ 可以照抄

- `init_yolov8_model` 整体结构（query 三连 + 存 attrs + 判断 fmt）
- `release_yolov8_model`（`free` + `rknn_destroy`）
- `inference` 里 `inputs_set → run → outputs_get → release` 的顺序

## ⚠️ 不能照抄

| 官方写的 | 为什么不行 |
|---|---|
| **`post_process()` 假设单个输出 `(84,8400)`** | ⭐ **我们的模型是两个输出**（coords+scores），后处理要用自己写的 |
| **`is_quant` 只看 `output_attrs[0]`** | 我们有两个输出，dtype 不同就判错 |
| **`convert_image_with_letterbox` 走 RGA** | RK3566 有 RGA 可以用，但**那是 P5 的优化项**。P4 先用 OpenCV，留干净的 baseline |

> **第 3 条特别注意：一开始就用 RGA，P5 那个"RGA 提升多少"的数字就永远测不出来。**

---

# 7. 下一步动作

**读 `official/yolov8.cc` 里的 `inference_yolov8_model()`，在每一行旁边标 `A` / `B` / `C`。**

- `A` 看不懂 → 去问
- `B` / `C` → 扫过去

**标完就能自己写出 `inference` 那一段了。**

---

# 8. 文件位置

```
~/projects/rknn/
├── yolov8n.onnx / yolov8n_split.onnx
├── yolov8n_int8.rknn      ★ 要部署的模型
├── compare_int8.py          Python 侧验证
└── cpp/
    ├── test_nms.cpp       ★ 已完成的后处理（iqo/nms/class_aware/decode）
    └── official/
        ├── main.cc        入口 + 收尾
        ├── yolov8.cc      ★ init / inference / release
        └── yolov8.h       context 结构定义
```

GitHub：`noSakiForSoyo/cppLearning`，最新 commit `84ff038`
