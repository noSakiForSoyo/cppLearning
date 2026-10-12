from rknn.api import RKNN

rknn = RKNN(verbose = False)

rknn.config(
  mean_values=[[0,0,0]],
  std_values=[[255,255,255]],
  target_platform = "rk3566",
  quantized_dtype = "w8a8",
  quantized_algorithm = 'normal'
)

print("load:",rknn.load_onnx(model="yolov8n.onnx"))
print("build:",rknn.build(do_quantization=True,dataset='calib.txt'))
print("export:",rknn.export_rknn('yolov8n_int8.rknn'))

print("analyze:", rknn.accuracy_analysis(
    inputs=['test.jpg'],
    output_dir='snapshot',
))
rknn.release()
print("done")