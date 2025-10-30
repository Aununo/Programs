from pwn import *

hex_str = '0e0b213f26041e480b26217f27342e175d0e070a3c5b103e2526217f27342e175d0e077e263451150104'
bytes_str = bytes.fromhex(hex_str)
print(bytes_str)
prefix = b'crypto{'

print(xor(bytes_str, prefix))
# b'myXORke+y_Q\x0bHOMe$~seG8bGURN\x04DFWg)a|\x1dTM!an\x7f'

key = b'myXORkey'

flag = xor(bytes_str, key).decode()
print(flag)
# crypto{1f_y0u_Kn0w_En0uGH_y0u_Kn0w_1t_4ll}