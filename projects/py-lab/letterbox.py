import cv2
import numpy as np
def letterbox(img,size = 640,color=(114,114,114)):
    h,w = img.shape[:2]
    scale = size/max(h.w)
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
