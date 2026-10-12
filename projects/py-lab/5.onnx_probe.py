import onnxruntime as ort
import numpy as np
import cv2
import torch
from NMS import NMS

from ultralytics import YOLO
def cxcywh2xyxy(b):
    x1 = b[0,:] - b[2,:]/2
    y1 = b[1,:] - b[3,:]/2
    x2 = b[0,:] + b[2,:]/2
    y2 = b[1,:] + b[3,:]/2
    return np.stack([x1,y1,x2,y2])

def letterbox(img,size = 640,color=(114,114,114)):
    h,w = img.shape[:2]
    scale = size/max(h,w)
    new_h,new_w = int(round(h*scale)),int(round(w*scale))
    resized = cv2.resize(img,(new_w,new_h))
    pad_w = (size - new_w) // 2
    pad_h = (size - new_h) // 2
    out = cv2.copyMakeBorder(resized, pad_h, size - new_h - pad_h,
                             pad_w, size - new_w - pad_w,
                             cv2.BORDER_CONSTANT, value=color)
    return out, scale, pad_w, pad_h
def unletterbox(box,scale,pad_w,pad_h):
    x1 = (box[0] - pad_w) / scale
    y1 = (box[1] - pad_h) / scale
    x2 = (box[2] - pad_w) / scale
    y2 = (box[3] - pad_h) / scale
    return np.array([x1,y1,x2,y2])

names = YOLO("yolov8n.pt").names       
sess = ort.InferenceSession("yolov8n.onnx",providers=['CPUExecutionProvider'])
for i in sess.get_inputs():
    print(i.name, i.shape, i.type)
for o in sess.get_outputs():
    print(o.name, o.shape, o.type)
x = np.random.rand(1,3,640,640).astype(np.float32)
out = sess.run(None, {sess.get_inputs()[0].name:x})

print("输出结果:",len(out),out[0].shape)

img = cv2.imread("test.jpg")
img = letterbox(img)[0]
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
x = img.transpose(2,0,1)[None].astype(np.float32)/255.0
out = sess.run(None, {"images":x})[0]

pred = out[0]
b_cxcywh = pred[:4,:]

scores = pred[4:,:]
conf = scores.max(axis=0)
cls = scores.argmax(axis=0)

mask = conf>0.25
print("检测到的目标数量:",int(mask.sum()))

for c,k in zip(conf[mask], cls[mask]):
    print("类别:",int(k),"置信度:",float(c))
kept = b_cxcywh[:,mask]
print(kept[:,:5])
xyxy = cxcywh2xyxy(kept).T
boxes_t = torch.from_numpy(xyxy).float()       
scores_t = torch.from_numpy(conf[mask]).float()

import torchvision
keep = torchvision.ops.nms(boxes_t, scores_t, iou_threshold=0.5)
print("torchvision NMS后剩余目标数量:", len(keep))
keep = NMS(boxes_t, scores_t, thresh=0.5)
print("NMS后剩余目标数量:",len(keep))