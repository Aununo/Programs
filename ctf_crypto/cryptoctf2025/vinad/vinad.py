from Crypto.Util.number import *
from output import *

def parinad(n):
        return bin(n)[2:].count('1') % 2

def vinad(x, R):
        return int(''.join(str(parinad(x ^ r)) for r in R), 2)

p = vinad(1, R)
q = n // p
e = vinad(1, R)
m = pow(c, inverse(e, (p-1)*(q-1)), n)
m = (m - sum(R)) % n
print(long_to_bytes(m))
