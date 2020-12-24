vec = require "vectorize"

a = vec {1, 2, 3}
print(a)

b = vec(3)
b[1] = -1
b[2] = 0
b[3] = 10
print(b)
print()

print(a:hadamard(b))
print(a * b)
print()

print(2 * a)
print(a:scale(2))
print()

x = vec.basis(3, 1)
print(vec.basis(3, 2))
