import numpy as np



a = np.zeros((3, 4))
print(a.shape, a.ndim, a.size, a.dtype)

b = np.arange(12).reshape(3, 4)
print(b.shape)
print(b[0, 1])
print(b[0:2, 1:3])
print(b[0])
print(b[:, 1])
print(b)
print(".......................................")

sub = a[:2, :2]
print(sub)
sub2 = a[:2, :2].copy()
sub[0, 0] = 100
print(a)
print(sub2)


print(".......................................")

img = np.full((2,3,3), 255, dtype=np.uint8)
bias = np.array([10, 20, 30], dtype=np.uint8)
print(img)
print(bias)
print((img + bias)[0])

a = np.array(250,dtype=np.uint8);
print(a+10)
print(np.clip(a.astype(np.int16) + 10, 0, 255).astype(np.uint8))