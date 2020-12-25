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
print()

print(a / b)
print(a / 2)
print()

print(1 / vec {0, 1, 2})
print()

x = vec {1, 2, 3}
y = vec {2, 0.1, -2}
print(x ^ y)
print(y ^ x)
print(x ^ 2)
print(2 ^ x)

print()

t = vec.linspace(-math.pi, math.pi, 25)
print(t)
print(t - (t^3)/6 + (t^5)/120 - (t^7)/5040 + (t^9)/362880)
