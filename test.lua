vec = require "vectorize"

a = vec{1, 2, 3}
b = vec{-1, 1, -2}

print(a^b)
print(a:pow_into(b))
print(a:pow_into(b))
print(a:pow_into(b))
print(a:pow_into(b))
b:pow_into(-1)
print(a:pow_into(b))
print(a:pow_into(b))
print(a:pow_into(b))
print(a:pow_into(b))
