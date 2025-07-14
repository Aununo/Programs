from Crypto.Util.number import *
from hashlib import sha256

N = 2**32
X0, X1, X2 = 1191871952, 424959397, 2071728008
a = (X2 - X1) * inverse(X1 - X0, N) % N
b = (X1 - a * X0) % N
X = X0

def gen():
    global X
    X = (X * a + b) % N
    return X

n = 10
lst = [X0]
for i in range(1, n):
    lst.append(gen())

flag = "cnss{" + sha256(str(lst).encode()).digest().hex() + "}"
print(flag)
"""
cnss{fb38e6d49d90503c213f9c24afe5f805866f0047b781fb5cc4f58c5ad8c09885}
"""