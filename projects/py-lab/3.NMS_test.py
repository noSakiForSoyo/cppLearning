from pathlib import Path

from ultralytics import YOLO
import torch

def iou_of_one_vs_many(a,b,eps=1e-7):
  x1 = torch.maximum(a[0], b[:,0])
  y1 = torch.maximum(a[1], b[:,1])
  x2 = torch.minimum(a[2], b[:,2])
  y2 = torch.minimum(a[3], b[:,3])
  inter = (x2-x1).clamp(min=0) * (y2-y1).clamp(min=0)
  area_a = (a[2]-a[0]) * (a[3]-a[1])
  area_b = (b[:,2]-b[:,0]) * (b[:,3]-b[:,1])
  return inter / (area_a + area_b - inter + eps)
def NMS(boxes,scores,thresh=0.5):
  order = scores.argsort(descending=True)
  keep = []
  while order.numel() > 0:
    i = order[0].item()
    keep.append(i)
    if(order.numel() == 1):
      break
    rest = order[1:]
    ious = iou_of_one_vs_many(boxes[i],boxes[rest])
    order = rest[ious <= thresh]
  return torch.tensor(keep,dtype=torch.long)

model = YOLO(str("yolov8n.pt"))
r = model("test.jpg", conf=0.25, iou=0.99)[0]
print("检测到", len(r.boxes), "个目标")

keep = NMS(r.boxes.xyxy, r.boxes.conf, thresh=0.5)
print("NMS后剩余", len(keep), "个目标")

rr = model("test.jpg", conf=0.25, iou=0.5)[0]
print("NMS后剩余", len(rr.boxes), "个目标")