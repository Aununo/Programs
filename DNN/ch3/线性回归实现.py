import random 
import torch 
from d2l import torch as d2l 

#生成合成数据集 
""" 
创建一个我们预先知道“正确答案”的人工数据集。这样一来，我们就可以用这个数据集来训练我们的模型，然后看看模型学习到的答案和我们预设的“正确答案”有多接近，从而判断模型的好坏。 
""" 

def synthetic_data(w, b, num_examples): #@save 
    """生成 y = Xw + b + 噪声""" 
    X = torch.normal(0, 1, (num_examples, len(w))) #特征,1000 × 2 (1000, 2) 
    y = torch.matmul(X, w) + b #标签 
    y += torch.normal(0, 0.01, y.shape) #添加噪声 
    return X, y.reshape((-1, 1))    



true_w = torch.tensor([2, -3.4]) #权重 2 × 1 (2, -3.4)^T 
true_b = 4.2 #偏置(bias) 
features, labels = synthetic_data(true_w, true_b, 1000) 
print('features:', features[0],'\nlabel:', labels[0]) 
d2l.set_figsize() 
d2l.plt.scatter(features[:, (1)].detach().numpy(), labels.detach().numpy(), 1); 
#d2l.plt.show()  



 # 读取数据集 

"""该函数接收批量大小、特征矩阵和标签向量作为输入生成大小为batch_size的小批量。 每个小批量包含一组特征和标签。""" 

def data_iter(batch_size, features, labels): 

     num_examples = len(features) 

     indices = list(range(num_examples)) 

     # 这些样本是随机读取的，没有特定的顺序 

     random.shuffle(indices) 

     for i in range(0, num_examples, batch_size): 

         batch_indices = torch.tensor( 

             indices[i: min(i + batch_size, num_examples)]) 

         yield features[batch_indices], labels[batch_indices] 



batch_size = 10 #批量大小 

for X, y in data_iter(batch_size, features, labels): 

     print(X, '\n', y) 

     break #只打印第一个小批量 



 #初始化模型参数 

w = torch.normal(0, 0.01, size=(2,1), requires_grad=True) 

b = torch.zeros(1, requires_grad=True) 



 #定义模型 

def linreg(X, w, b):  #@save 

     """线性回归模型""" 

     return torch.matmul(X, w) + b 



 #定义损失函数 

def squared_loss(y_hat, y):  #@save 

     """均方损失""" 

     return (y_hat - y.reshape(y_hat.shape)) ** 2 / 2 



 #定义优化算法 

def sgd(params, lr, batch_size):  #@save 

     """小批量随机梯度下降""" 

     with torch.no_grad(): 

         for param in params: 

             param -= lr * param.grad / batch_size 

             param.grad.zero_() 



 #训练模型 



def train(lr, num_epochs, batch_size):  #@save   

     """训练模型""" 

     net, loss = linreg, squared_loss 

     for epoch in range(num_epochs): 

         for X, y in data_iter(batch_size, features, labels): 

             l = loss(net(X, w, b), y)  #计算损失 

             l.sum().backward()  #计算梯度 

             sgd([w, b], lr, batch_size)  #更新参数 

         with torch.no_grad(): 

             train_l = loss(net(features, w, b), labels) 

             print(f'epoch {epoch + 1}, loss {float(train_l.mean()):f}') 



lr, num_epochs = 0.03, 3 #学习率和迭代次数 

train(lr, num_epochs, batch_size) 





 #打印参数 

print(f'error in estimating w: {true_w - w.reshape(true_w.shape)}') 

print(f'error in estimating b: {true_b - b}')    





 #绘制拟合曲线 

d2l.plt.scatter(features[:, (1)].detach().numpy(), labels.detach().numpy(), 1); 

x = torch.arange(-3.0, 3.0, 0.01, requires_grad=False) 

d2l.plt.plot(x.detach().numpy(), (true_w[0] * x + true_b).detach().numpy(), 'r', lw=2 ) 

d2l.plt.plot(x.detach().numpy(), (w[0] * x + b).detach().numpy(), 'g--', lw=2) 

d2l.plt.xlabel 

('x1') 

d2l.plt.ylabel('y') 

d2l.plt.legend(['true line', 'predicted line']) 

d2l.plt.show() #显示图形 

 # 以上代码实现了线性回归的基本流程，包括数据生成、模型定义、损失函数、优化算法以及训练过程。 

 # 通过训练，我们可以看到模型学习到了接近真实参数的值，并且拟合曲线与真实数据点的分布相符。