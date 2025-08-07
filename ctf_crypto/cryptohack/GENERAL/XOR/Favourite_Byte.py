from pwn import *

hex_str = '73626960647f6b206821204f21254f7d694f7624662065622127234f726927756d'
bytes_str = bytes.fromhex(hex_str)

for i in range(256):
    try:
        flag = xor(bytes_str, i).decode()
        print(f'Key: {i}, Flag: {flag}')
    except UnicodeDecodeError:
        continue
    except Exception as e:
        print(f'Key: {i}, Error: {e}')
        continue
print("Done!")

# Key: 16, Flag: crypto{0x10_15_my_f4v0ur173_by7e}