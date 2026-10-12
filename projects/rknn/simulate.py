import numpy as np
import cv2
import torch
import onnxruntime as ort
from rknn.api import RKNN


IMG = "test.jpg"
SIZE = 640

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

img0 = cv2.imread(IMG)
img = letterbox(img0)[0]
img = cv2.cvtColor(img,cv2.COLOR_RGB2BGR)
x = img[None]
x_onnx = img.transpose(2,0,1)[None].astype(np.float32)/255.0
print("输入：" ,x.shape,x.dtype)

sess = ort.InferenceSession("yolov8n.onnx",providers=['CPUExecutionProvider'])
out_onnx = sess.run(None,{'images':x_onnx})[0]
print(out_onnx.shape,"-",out_onnx.min(),"-",out_onnx.max())

rknn = RKNN(verbose = False)
rknn.config(
    mean_values=[[0, 0, 0]],
    std_values=[[255, 255, 255]],
    target_platform='rk3566',
)

print("load_onnx :", rknn.load_onnx(model='yolov8n.onnx'))
print("build     :", rknn.build(do_quantization=False))
print("init_rt   :", rknn.init_runtime())        
outputs = rknn.inference(inputs=[x])
out = outputs[0]
print(out_onnx.shape,"-",out_onnx.min(),"-",out_onnx.max())
print(out.shape,"-",out.dtype,"-",out.min(),"-",out.max())
rknn.release()

a = out_onnx[0] if out_onnx.ndim == 3 else out_onnx
b = out[0] if out.ndim == 3 else out

d_coord = np.abs(a[:4] - b[:4])      
d_score = np.abs(a[4:] - b[4:])      

print("坐标行最大差异:", d_coord.max())
print("分数行最大差异:", d_score.max())

