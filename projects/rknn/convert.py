from rknn.api import RKNN

rknn = RKNN(verbose = True)

rknn.config(
  mean_values = [[0,0,0]],
  std_values = [[255,255,255]],
  target_platform = 'rk3566',
)

ret = rknn.load_onnx(model='yolov8n.onnx')

assert ret == 0 , "load_onnx 失败"

ret = rknn.build(do_quantization=False)
assert ret == 0 , "build 失败"

ret = rknn.export_rknn("yolov8n_fp16.rknn")
assert ret == 0 , "export 失败"

print("\n 转化完成")
