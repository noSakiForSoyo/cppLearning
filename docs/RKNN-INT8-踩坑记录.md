# RKNN INT8 量化踩坑记录 —— 输出张量拆头

> 平台：RK3566 / rknn-toolkit2 2.3.2 / YOLOv8n
> 记录时间：P3 收尾阶段
> 结论：**这是一个已知的、平台无关的问题**，参考 [这篇](https://ai6s.net/6ab1d14005257b08571228bc.html)

---

## 一、症状：一种"安静的失败"

| 模式 | 结果 |
|---|---|
| **FP16** | ✅ 完全正常 —— 3 个框，最高置信度 0.7715，与 onnxruntime 的 cos 差异 0.9997 |
| **INT8** | ❌ **分类分数全部精确为 0**，0 个框 |

**而坐标部分是好的**（范围 0 ~ 641.89，是合理的 640 坐标系值）。

> **最折磨人的地方：没有任何报错。**
> 转换成功、推理成功、速度正常、形状也对 —— 只是结果全空。
> 如果直接上板才发现，会先怀疑模型、怀疑数据、怀疑驱动，最后才怀疑量化。

---

## 二、根因：坐标和置信度**共用一把量化尺子**

`output0` 是 `(1, 84, 8400)`，里面混着**量级差 640 倍**的两种数据：

```
前  4 行：框坐标     范围 0 ~ 640
后 80 行：类别置信度  范围 0 ~ 1
```

**INT8 per-tensor 量化给【整个张量】算一个统一的 scale：**

```
scale ≈ 最大值 / 127 ≈ 640 / 169 ≈ 3.7956        ← 实测值
```

**scale 由数值范围最大的【坐标】决定。然后置信度被怎么处理：**

```
置信度 0.7496  →  round(0.7496 / 3.7956) = round(0.197) = 0     ❌
置信度 0.4150  →  round(0.4150 / 3.7956) = round(0.109) = 0     ❌
```

**⇒ 所有置信度全部量化成 0 → 后处理按阈值一过滤 → 一个框都不剩。**

> **不是模型瞎了，是置信度在量化那一刻就死了。**
> **坐标通道几乎不受影响（损失 1~2 像素精度）——
> 所以这个问题极具迷惑性：框的"骨架"还在，只是分数没了。**

---

## 三、排查三步法（**可复用到任何 NPU 平台**）

| 步 | 做什么 | 为什么 |
|---|---|---|
| **1** | **FP16 对照** —— 同一模型转不量化的版本跑一遍 | 成本最低，直接定位"是不是量化的锅" |
| **2** | **dump 原始输出**，跳过后处理 | 后处理有阈值过滤，**会掩盖真相** |
| **3** | **看数值范围 / 量化步长** | 真凶现形 |

### ⭐ 第三步的一个更直接的技巧

**不要只看 min/max —— 看【相邻取值的差值】：**

```python
p = out[0] if out.ndim == 3 else out
u = np.unique(p[:4, :])          # 坐标部分的所有不同取值
print("相邻差值:", np.diff(u[:11]))
```

**如果输出是一串"等间距"的值**（例如 `3.7955956 3.7955961 3.7955952 ...`），
**那个间距就是量化 scale 本身。**

> **这比看 min/max 更直接** —— min/max 只能推出量程，看差值能直接看到尺子有多粗。

---

## 四、解法：**把输出头拆开，各用各的 scale**

### 方案 A：在 ONNX 末尾加一个 Split（我们用的）

```python
import onnx
from onnx import helper, TensorProto

model = onnx.load('yolov8n.onnx')
g = model.graph
old_name = g.output[0].name          # 'output0'

node = helper.make_node(
    'Split', inputs=[old_name], outputs=['coords', 'scores'],
    axis=1, split=[4, 80], name='Split_out',
)
g.node.append(node)

del g.output[:]
g.output.extend([
    helper.make_tensor_value_info('coords', TensorProto.FLOAT, [1,  4, 8400]),
    helper.make_tensor_value_info('scores', TensorProto.FLOAT, [1, 80, 8400]),
])

onnx.checker.check_model(model)
onnx.save(model, 'yolov8n_split.onnx')
```

### 方案 B：找末尾的 `Concat`，把它的**输入**直接提升为图输出

（更"原生"，不用新增算子 —— 见参考文章）

### 推理侧：合并回去，后处理零改动

```python
def merge_outputs(outputs):
    boxes  = [o for o in outputs if o.ndim == 3 and o.shape[1] == 4]
    others = [o for o in outputs if not (o.ndim == 3 and o.shape[1] == 4)]
    return np.concatenate(boxes + others, axis=1)
```

> **设计原则：格式适配集中在"拆/合"两个函数里，
> 后处理和业务代码完全不感知 RKNN 的存在。**

---

## 五、验证结果

| | FP16 | INT8（拆头后） |
|---|---|---|
| 分数最大值 | 0.7715 | **0.7496** |
| 分数量化步长 | — | **0.00293975**（细了 **1290 倍**） |
| 坐标量化步长 | — | 2.8027 |
| 框数 | 3 | 4 |
| 共同框坐标差 | — | **< 3 像素** |
| 共同框置信度差 | — | **~0.02** |

> **拆完后，分数拿到了自己的尺子：`0.7496 / 0.00293975 = 255.0` ——
> 正好把 int8 的 255 档全部用上。**

---

## 六、被排除的方向（**别在这上面浪费时间**）

| 假设 | 怎么测 | 结果 |
|---|---|---|
| 输入归一化配对错 | `std=255`+uint8 vs `std=1`+float32 | ❌ 都全 0 |
| 校准集不匹配 | 游戏截图 / test.jpg×10 / Windows 壁纸 | ❌ 都全 0 |
| **权重量化粒度** | `quantized_method='channel'` | ❌ 全 0 |
| 量化算法 | `normal` / `mmse` | ❌ 全 0 |
| 官方 `convert.py` 有特殊处理 | 对比源码 | ❌ 和我们写的一模一样 |
| 权重的离群值（build 警告） | 60~113 倍离群值 | 🟡 **是干扰项，不是根因** |

### ⭐ 为什么 `per-channel` 也救不了

> **很多 NPU 工具链的【输出张量】只支持 per-tensor 量化。**
> `quantized_method='channel'` 管的是**权重**，管不到输出。
>
> **所以这条路从一开始就是死的** —— 参考文章也确认了这一点。

---

## 七、几个附带的坑（都是已知的）

| 坑 | 说明 |
|---|---|
| **`.rknn` 不能 load 回仿真器** | 顺序必须是 `build → init_runtime(target=None) → export_rknn` |
| **RKNN 会自动融合 SiLU** | 日志里的 `replace_exswish` = 把 56 组 `Sigmoid+Mul` 合成一个 `exSwish` |
| **opset ≤ 19** | rknn-toolkit2 的限制，更高的要先用 `onnx.version_converter` 降级 |
| **输出 dtype 会变 int8** | build 时会警告，部署时要注意（Runtime API 侧） |
| `E RKNN: Unkown op target: 0` | 反复出现但无害（FP16/INT8 都有） |

---

## 八、⭐ 能带走的经验

> **量级差大的数据，不要共用一把量化尺子。**
>
> YOLOv5 / v8 / v11 的输出结构**都有这个隐患**
> （单个张量里混着坐标 0~512 和置信度 0~1）。
>
> **凡是 per-tensor INT8 量化的 NPU 工具链，都可能踩到 ——
> 和平台无关，RKNN / 地平线 / 昇腾 都一样。**

### 一条方法论

> **"安静的失败"（不报错、但结果全空）的通用排查顺序：**
> 1. **浮点对照**（定位是不是量化的锅）
> 2. **dump 原始输出**（跳过后处理，它藏真相）
> 3. **看数值分布 / 量化步长**（找到真凶）

---

## 九、参考

- [YOLOv8 转 RKNN INT8 量化后置信度全变 0？把输出头拆开就好了](https://ai6s.net/6ab1d14005257b08571228bc.html)
- [LdDl/rv1106-yolov8 —— 另一个平台的同类处理](https://github.com/LdDl/rv1106-yolov8)
