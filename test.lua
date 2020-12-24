vec = require "vectorize"
print(vec.new)
a = vec(3)
print(a[1])
a[1] = a[1] * a[2] + 1
print(a)
