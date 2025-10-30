#sage
from Crypto.Util.number import *

def highlow(x):
    return (x >> 32, x % (2 ** 32))

init = b'cnss{I_a'
m0 = bytes_to_long(init)
cipher = [4840951631397558164, 5492303526413306583, 
6271460196030786735, 6127905759336302986, 601209385465514967]
MASK2 = 16500653344889503744

c0 = cipher[0]
hm0, lm0 = highlow(m0)
hc0, lc0 = highlow(c0)

hk = hc0 ^^ lm0 ^^ (MASK2 >> 32)
lk = lc0 ^^ hm0

key = (hk << 32) + lk
flag = ''

for i in cipher:
    l, h = highlow(i ^^ MASK2 ^^ key)
    flag += long_to_bytes((h << 32) | l).decode()
    
print(flag)