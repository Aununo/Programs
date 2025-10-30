import requests

url = 'http://aes.cryptohack.org/block_cipher_starter'
req = requests.get(url + '/encrypt_flag/')
ciphertext = req.json()["ciphertext"]

req = requests.get(url + '/decrypt/' + ciphertext + '/')
m = req.json()["plaintext"]
print(bytes.fromhex(m).decode())