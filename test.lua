vec = require "vectorize"

a = vec {1, 2, 3}
print(a)
for i, v in a:iter() do
    print(i, v)
end

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

print(vec.linspace(-1, 1, 11):exp())
print(vec.linspace(0, 2, 11):ln())
print(vec.linspace(0, 2, 11):ln1p())

print(vec.linspace(-math.pi, math.pi, 5):sin():asin())
print(vec.linspace(-math.pi, math.pi, 5):cos():acos())
