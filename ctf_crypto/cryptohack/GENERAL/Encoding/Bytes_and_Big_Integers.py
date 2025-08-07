from Crypto.Util.number import *

m = 11515195063862318899931685488813747395775516287289682636499965282714637259206269
flag = long_to_bytes(int(m)).decode()
print(flag)
"""
crypto{3nc0d1n6_4ll_7h3_w4y_d0wn}
"""