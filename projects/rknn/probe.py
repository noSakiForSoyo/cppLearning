import numpy as np
import cv2
from rknn.api import RKNN

# ---------- 预处理 ----------
img0 = cv2.imread("test.jpg")
h, w = img0.shape[:2]
s = 640 / max(h, w)
r = cv2.resize(img0, (int(round(w*s)), int(round(h*s))))
pw = (640 - r.shape[1]) // 2
ph = (640 - r.shape[0]) // 2
p = cv2.copyMakeBorder(r, ph, 640 - r.shape[0] - ph,
                       pw, 640 - r.shape[1] - pw,
                       cv2.BORDER_CONSTANT, value=(114, 114, 114))
x = cv2.cvtColor(p, cv2.COLOR_BGR2RGB)[None]      # uint8

# ---------- 构建 INT8 ----------
k = RKNN(verbose=False)
k.config(mean_values=[[0, 0, 0]], std_values=[[255, 255, 255]],
         target_platform='rk3566')
k.load_onnx(model='yolov8n_split.onnx')
k.build(do_quantization=True, dataset='calib.txt')
k.init_runtime()

outs = k.inference(inputs=[x])
k.release()

# ---------- 把两个输出全打印出来 ----------
print("=" * 60)
print("输出个数:", len(outs))
for i, o in enumerate(outs):
    print(f"\n--- out[{i}] ---")
    print("  shape :", o.shape, "  dtype :", o.dtype)
    print("  min/max:", float(o.min()), "~", float(o.max()))
    u = np.unique(np.ravel(o))
    print("  不同取值个数:", len(u))
    print("  最小 10 个  :", u[:10])
    if len(u) > 1:
        print("  相邻差值(前5):", np.diff(u[:6]))
print("=" * 60)