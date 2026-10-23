import numpy as np
import cv2
import torch
from rknn.api import RKNN

def letterbox(img, size=640, color=(114, 114, 114)):
    h, w = img.shape[:2]
    scale = size / max(h, w)
    new_w, new_h = int(round(w * scale)), int(round(h * scale))
    resized = cv2.resize(img, (new_w, new_h))
    pad_w = (size - new_w) // 2
    pad_h = (size - new_h) // 2
    out = cv2.copyMakeBorder(resized, pad_h, size - new_h - pad_h,
                             pad_w, size - new_w - pad_w,
                             cv2.BORDER_CONSTANT, value=color)
    return out, scale, pad_w, pad_h


def iou_one_vs_many(a, b, eps=1e-7):
    x1 = torch.maximum(a[0], b[:, 0]); y1 = torch.maximum(a[1], b[:, 1])
    x2 = torch.minimum(a[2], b[:, 2]); y2 = torch.minimum(a[3], b[:, 3])
    inter = (x2 - x1).clamp(min=0) * (y2 - y1).clamp(min=0)
    area_a = (a[2] - a[0]) * (a[3] - a[1])
    area_b = (b[:, 2] - b[:, 0]) * (b[:, 3] - b[:, 1])
    return inter / (area_a + area_b - inter + eps)


def nms(boxes, scores, thresh):
    order = scores.argsort(descending=True)
    keep = []
    while order.numel() > 0:
        i = order[0].item()
        keep.append(i)
        if order.numel() == 1:
            break
        rest = order[1:]
        order = rest[iou_one_vs_many(boxes[i], boxes[rest]) <= thresh]
    return keep
#按类别做nms
def nms_class_aware(boxes, scores, cls, thresh):
    out = []
    for c in cls.unique():
        idx = torch.where(cls == c)[0]
        k = nms(boxes[idx], scores[idx], thresh)
        out.append(idx[k])
    if len(out) == 0:                                  
        return torch.empty(0, dtype=torch.long)
    return torch.cat(out)
def fix_shape(a,C):
    a = np.asarray(a)
    if a.ndim == 3:
        a=a[0]
    if a.shape[0]==C:
        return a
    if a.shape[1]==C:
        return a.T
    
def decode(outs ,conf_thres=0.25,iou_thres=0.7):
    b_cxcywh = fix_shape(outs[0],4)
    scores = fix_shape(outs[1],80)
    conf = scores.max(axis=0)
    cls = scores.argmax(axis=0)
    mask = conf>conf_thres
    kept = b_cxcywh[:,mask]
    cx,cy,w,h = kept[0],kept[1],kept[2],kept[3]
    xyxy = np.stack([cx-w/2,cy-h/2,cx+w/2,cy+h/2],axis=1)
    boxes_t = torch.from_numpy(xyxy).float()
    scores_t = torch.from_numpy(conf[mask]).float()
    cls_t = torch.from_numpy(cls[mask].astype(np.int64))
    keep = nms_class_aware(boxes_t,scores_t,cls_t,iou_thres)
    return boxes_t[keep],scores_t[keep],cls_t[keep]

img0 = cv2.imread("test.jpg",cv2.IMREAD_COLOR)
img = letterbox(img0,640)[0]
img = cv2.cvtColor(img,cv2.COLOR_BGR2RGB)
x = img[None]

def build_and_run(do_quant,name):
    rknn = RKNN(verbose=False)
    rknn.config(
        mean_values = [[0,0,0]],
        std_values = [[255,255,255]],
        target_platform = 'rk3566',
        quantized_dtype='w8a8',
        quantized_method='channel',         
        quantized_algorithm='normal',  
        
    )
    print("load_onnx :", rknn.load_onnx(model='yolov8n_split.onnx'))
    if(do_quant):
        rknn.build(do_quantization=True,dataset='calib.txt')
    else:
        rknn.build(do_quantization=False)
    rknn.init_runtime()
    outs = rknn.inference(inputs=[x])
    outs[0].astype(np.float32).tofile('cpp/coords.bin')
    outs[1].astype(np.float32).tofile('cpp/scores.bin')
    print("输出个数:", len(outs))

    scores = fix_shape(outs[1], 80)
    conf_all = scores.max(axis=0)

    
    boxes, confs, clses = decode(outs,conf_thres=0.25)        
    print("分类分数最大值:", float(conf_all.max()))

    print(
        "分类分数 Top 5:",
        np.sort(conf_all)[::-1][:5]
    )
    print(
        "高于 0.25 的候选框:",
        int(np.count_nonzero(conf_all > 0.25))
    )
    print("框数：",len(boxes))
    for b,c,k in zip(boxes,confs,clses):
        print(f" conf={c}  "
              f"box=({b[0]},{b[1]},{b[2]},{b[3]})")
        
    rknn.release()
    return boxes,confs,clses

print("\n>>> 构建 FP16 ...")
b16, c16, k16 = build_and_run(False, "FP16")

print("\n>>> 构建 INT8 ...")
b8, c8, k8 = build_and_run(True, "INT8")

print("\n" + "=" * 50)
print(f"FP16 框数: {len(b16)}    INT8 框数: {len(b8)}")
if len(b16) == len(b8):
    print("✅ 数量一致")
    for i in range(len(b16)):
        print(f"  框{i}: FP16 conf={c16[i]} cls={k16[i]}  |  "
              f"INT8 conf={c8[i]} cls={k8[i]}")