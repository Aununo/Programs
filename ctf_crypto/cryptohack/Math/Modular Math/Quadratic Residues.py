#Sympy.is_quad_residue, is_square(sage)

def quadratic_residues(x, p):
    for i in range(1, p):
        if i * i % p == x:
            return i
    return False

print(quadratic_residues(14, 29))
print(quadratic_residues(6, 29))
print(quadratic_residues(11, 29))

#sage
p = 29
F = GF(p)

ords = [14, 6, 11]
for o in ords:
    print(F(o).is_square())