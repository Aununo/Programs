# exp 1

from Crypto.Util.number import long_to_bytes

class LFSR():
    def __init__(self, seed, mask, length):
        self.length_mask = 2 ** length - 1
        self.mask = mask & self.length_mask
        self.state = seed & self.length_mask

    def next(self):
        next_state = (self.state << 1) & self.length_mask
        i = self.state & self.mask & self.length_mask
        output = 0
        while i != 0:
            output ^= (i & 1)
            i = i >> 1
        next_state ^= output
        self.state = next_state
        return output

    def getrandbit(self, nbit):
        output = 0
        for _ in range(nbit):
            output = (output << 1) ^ self.next()
        return output

KNOWN_SEED = 37285
KNOWN_LENGTH = 16
CIPHER_FILE = 'cipher.enc'
KNOWN_PLAINTEXT_PREFIX = b'cnss{' 

# 读取密文文件
with open(CIPHER_FILE, 'rb') as f:
    ciphertext = f.read()

print("[-] 开始暴力破解mask...")

# 遍历所有可能的16位mask
for potential_mask in range(2 ** KNOWN_LENGTH):
    
    # 为了防止打印过多内容，每1000次打印一次进度
    if potential_mask % 1000 == 0:
        print(f"[*] 正在尝试 mask = {potential_mask}")

    # 使用已知的seed和猜测的mask初始化LFSR
    lfsr_instance = LFSR(KNOWN_SEED, potential_mask, KNOWN_LENGTH)
    
    decrypted_prefix = b''
    
    # 尝试解密文件的前几个字节，数量和我们的已知明文前缀一样多
    for i in range(len(KNOWN_PLAINTEXT_PREFIX)):
        keystream_byte = lfsr_instance.getrandbit(8)
        decrypted_byte = ciphertext[i] ^ keystream_byte
        decrypted_prefix += long_to_bytes(decrypted_byte)
        
    # 检查解密出的前缀是否和我们猜测的一样
    if decrypted_prefix == KNOWN_PLAINTEXT_PREFIX:
        print(f"\n[+] 找到了正确的mask! mask = {potential_mask}")
        
        # 用找到的正确mask重新初始化LFSR
        lfsr_final = LFSR(KNOWN_SEED, potential_mask, KNOWN_LENGTH)
        
        # 解密完整文件
        plaintext = b''
        for ch in ciphertext:
            c = ch ^ lfsr_final.getrandbit(8)
            plaintext += long_to_bytes(c)
            
        print(f"[+] 解密成功! Flag: {plaintext.decode()}")
        break 
else:
    print("\n[-] 破解失败，没有找到正确的mask。")


# ---------------------------------------------------------------------------------


from z3 import *
from Crypto.Util.number import *

def reducer(x):
    x = (x >> 16) ^ x
    x = (x >> 8) ^ x
    x = (x >> 4) ^ x
    x = (x >> 2) ^ x
    x = (x >> 1) ^ x
    return x & 1

class LFSR():
    def __init__(self, seed, mask, length):
        self.length_mask = 2 ** length - 1
        self.mask = mask & self.length_mask
        self.state = seed & self.length_mask

    def next(self):
        next_state = (self.state << 1) & self.length_mask
        i = self.state & self.mask & self.length_mask
        lastbit = reducer(i)
        next_state ^= lastbit
        self.state = next_state
        return lastbit

    def getrandbit(self, nbit):
        output = 0
        for _ in range(nbit):
            output = (output << 1) ^ self.next()
        return output

def check(mask, cipher, sol):
    seed = 37285
    lfsr = LFSR(seed, mask, 16)
    flag = [99, 110, 115, 115]  # b'cnss'
    for i in range(len(flag)):
        c = flag[i] ^ lfsr.getrandbit(8)
        sol.add(c == cipher[i])
    return 1

cipher = []
with open('cipher.enc', 'rb') as f:
    for i in f.read():
        cipher.append(i)

mask = BitVec('mask', 16)
sol = Solver()
check(mask, cipher[:4], sol) 

print("Here")
sat_result = sol.check()
print(sat_result)
model_result = sol.model()
print(model_result)

def decrypt(lfsr, cipher):
    flag = b''
    for ch in cipher:
        c = ch ^ lfsr.getrandbit(8)
        flag += long_to_bytes(c)
    print(flag)

if sat_result == sat:
    found_mask = model_result[mask].as_long()
    print(f"\n[+] 成功找到 mask: {found_mask}")
    
    seed = 37285
    lfsr_for_decryption = LFSR(seed, found_mask, 16)
    
    print("[+] 正在解密，flag是:")
    decrypt(lfsr_for_decryption, cipher)
else:
    print("\n[-] Z3未能找到mask的解。")