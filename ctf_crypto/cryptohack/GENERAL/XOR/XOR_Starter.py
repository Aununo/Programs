#exp1.py
ords = [ord(c) for c in 'label']
def xor(ords, num):
    flag = "".join(chr(o ^ num) for o in ords)
    return flag
flag = xor(ords, 13)
print(f"crypto{{{flag}}}")

#exp2.py
from pwn import xor
flag = xor('label', 13).decode()
print(f"crypto{{{flag}}}") 
"""
crypto{aloha}
"""