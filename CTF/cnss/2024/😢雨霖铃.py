from Crypto.Util.number import *

n = int('0xa755cc2e436c157b484d286c8390f099c66a2cde756745ef9adac125d503e3ab931540e32cc30b20a8a90fe7b1e6df42559443cbe0a5b1efd71aa48bd76b4481d11a245557c258f17f8aa6dc1ac9c4d41f59afeb02f4f2a9450ebd77b70ffa46c050c69343247a5a7b4ee52f93f8b063526f1f1b0ad22cd01a7292902e6c140d', 16)
e = 65537

msg = b"Please keep away from me, I had to study hard"
m = bytes_to_long(msg)
c = hex(pow(m, e, n))
print(c)
# cnss{journey to the crypto begin}