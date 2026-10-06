from ultralytics import YOLO
import torch

model = YOLO("yolov8n.pt")
model.export(format="onnx",imgsz=640,opset=12)
