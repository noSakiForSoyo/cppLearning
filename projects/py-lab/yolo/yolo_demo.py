from ultralytics import YOLO

model = YOLO("yolov8n.pt") 
results = model("test.jpg")
results[0].save("result.jpg")
print(results[0].speed)
for conf in [0.9, 0.5, 0.25, 0.05]:
    r = model("test.jpg", conf=conf)[0]
    print(f"conf={conf:4.2f}  检测到 {len(r.boxes)} 个目标")
print(model.names)
print("检测到", len(results[0].boxes), "个目标")