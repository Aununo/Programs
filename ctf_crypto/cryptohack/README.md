### Codes for [CryptoHack](https://cryptohack.org/).

Register exp: caesar.py
```py
def caesar_cipher(text, shift, mode=''):
    if mode not in ['encrypt', 'decrypt']:
        raise ValueError("Mode should be either 'encrypt' or 'decrypt'")
    if mode == 'decrypt':
        shift = -shift
    
    result = []

    for char in text:
        if char.isalpha(): 
            start = ord('A') if char.isupper() else ord('a')
            offset = (ord(char) - start + shift) % 26
            result.append(chr(start + offset))
        else:
            result.append(char)
    
    return ''.join(result)

ct = 'ECOGTC JQUV QAUVGT TGITGV'

for i in range(1, 26):
    print(f"Shift {i}: {caesar_cipher(ct, i, 'decrypt')}")
"""
Shift 2: CAMERA HOST OYSTER REGRET
"""
```