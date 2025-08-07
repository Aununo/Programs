from Crypto.Cipher import AES
import hashlib
import requests

url = 'https://aes.cryptohack.org/passwords_as_keys/'
req = requests.get(url + '/encrypt_flag/')
ciphertext = req.json()["ciphertext"]
ciphertext = bytes.fromhex(str(ciphertext))

words = requests.get("https://gist.githubusercontent.com/wchargin/8927565/raw/d9783627c731268fb2935a731a618aa8e95cf465/words").text.splitlines()
for word in words:
    key = hashlib.md5(word.encode()).digest()
    cipher = AES.new(key, AES.MODE_ECB)
    try:
        plaintext = cipher.decrypt(ciphertext)
        if b'crypto{' in plaintext:
            print(plaintext.decode())
            break
    except:
        pass