vec = require "vectorize"

a = vec{1, 2, 3}
b = vec{-1, 1, -1}
print(a * b)
print(b * a)
print(a:hadamard_into(b))
print(a:div(3))
print(a:div(3))
print(a:mul_into(3))
print(a:mul_into(3))
print(a:mul_into(3))
print(a:mul_into(3))
