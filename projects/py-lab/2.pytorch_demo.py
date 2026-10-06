import torch
import torch.nn as nn

torch.manual_seed(0)
X = torch.randn(200, 1)
y = 2 * X + 1 + torch.randn(200, 1) * 0.1

model = nn.Linear(1, 1)

loss_fn = nn.MSELoss()
optimizer = torch.optim.SGD(model.parameters(), lr=0.01)

for epoch in range(1000):
    y_pred = model(X)
    loss = loss_fn(y_pred, y)

    optimizer.zero_grad()
    loss.backward()
    
    optimizer.step()
    if epoch % 100 == 0:
        print(f'Epoch {epoch}, Loss: {loss.item()}')
w = model.weight.item()
b = model.bias.item()
print(f'Learned parameters: w = {w}, b = {b}')