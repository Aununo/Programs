import random
import hashlib

p = Integer(
    "ffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024"
    "e088a67cc74020bbea63b139b22514a08798e3404ddef9519b3cd"
    "3a431b302b0a6df25f14374fe1356d6d51c245e485b576625e7ec"
    "6f44c42e9a637ed6b0bff5cb6f406b7edee386bfb5a899fa5ae9f"
    "24117c4b1fe649286651ece45b3dc2007cb8a163bf0598da48361"
    "c55d39a69163fa8fd24cf5f83655d23dca3ad961c62f356208552"
    "bb9ed529077096966d670c354e4abc9804f1746c08ca237327fff"
    "fffffffffffff", 16
)
g = Integer(2)

a = random.randint(1, p-1)
A = power_mod(g, a, p)

b = random.randint(1, p-1) 
B = power_mod(g, b, p)

s1 = power_mod(B, a, p)
s2 = power_mod(A, b, p)
assert s1 == s2, "The shared secret keys do not match!"

s_byte = str(s1).encode()
s = hashlib.sha256(s_byte).hexdigest()[:32] #128bit

print(f"s: {s}")

