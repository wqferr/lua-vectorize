vec = require "vectorize"

a = vec {1, 2, 3}
print(a)

b = vec(3)
b[1] = -1
b[2] = 0
b[3] = 10
print(b)

print(a:hadamard(b))
