vec = require "vectorize"
inspect = require "inspect"
print(vec.new)
a = vec(3)
a[1] = 1
a[2] = 2
a[3] = 3

b = vec(3)
b[1] = -1
b[2] = 0
b[3] = 10

print(a:hadamard(b))
