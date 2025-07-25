def Exgcd(a, b):
    if b == 0:
        return a, 1, 0
    d, x, y = Exgcd(b, a % b)
    return d, y, x - (a // b) * y

p = 26513
q = 32321
print(Exgcd(p, q))

#print(xgcd(p, q)) (1, 10245, -8404) sage