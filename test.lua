vec = require "vectorize"
inspect = require "inspect"
print(vec.new)
a = vec(3)
a[1] = 1
a[2] = 2

print(a)
print((a + 3) + 1)
-- print(inspect(getmetatable(a + a)))
print(a+a+a+2)
