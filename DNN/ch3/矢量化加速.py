#matplotlib inline
import math
import time
import numpy as np
import torch
from d2l import torch as d2l

## 我们实例化两个全为1的10000维向量。 在一种方法中，我们将使用Python的for循环遍历向量； 在另一种方法中，我们将依赖对+的调用。

n = 10000
a = torch.ones(n)
b = torch.ones(n)

# 运行时间的基准测试，所以我们定义一个计时器：
class Timer: #@save
    def __init__(self):
        self.times = []
        self.start() #构造函数
    #方法
    def start(self):
        self.tik = time.time()

    def stop(self):
        """停止计时器并将时间记录在times列表中"""
        self.times.append(time.time() - self.tik)
        return self.times[-1]
    
    def avg(self):
        """返回平均时间"""
        return sum(self.times) / len(self.times)
    
    def sum(self):
        """返回总时间"""
        return sum(self.times)
    
    def cumsum(self):
        """返回累积时间"""
        return np.array(self.times).cumsum().tolist()
    
# 1

c = torch.zeros(n)
timer = Timer()
for i in range(n):
    c[i] = a[i] + b[i]
print(f'1. Python for loop: {timer.stop():.5f} seconds') 

# 2
timer.start()
c = a + b
print(f'2. PyTorch vectorization: {timer.stop():.5f} seconds')
