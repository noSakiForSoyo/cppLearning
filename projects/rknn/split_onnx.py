import onnx
from onnx import helper, TensorProto

model = onnx.load('yolov8n.onnx')
g = model.graph

old_name = g.output[0].name
print("原输出:", old_name)

# 在 output0 后面接一个 Split：沿 axis=1 切成 [4, 80]
node = helper.make_node(
    'Split',
    inputs=[old_name],
    outputs=['coords', 'scores'],
    axis=1,
    split=[4, 80],
    name='Split_out',
)
g.node.append(node)

# 换掉输出声明：一个变两个
del g.output[:]
g.output.extend([
    helper.make_tensor_value_info('coords', TensorProto.FLOAT, [1,  4, 8400]),
    helper.make_tensor_value_info('scores', TensorProto.FLOAT, [1, 80, 8400]),
])

onnx.checker.check_model(model)
onnx.save(model, 'yolov8n_split.onnx')
print("saved: yolov8n_split.onnx")