local vec = require "vectorize"

t = vec.linspace(-1, 1, 21)
print(t)
print(t:square())
print(t:sqrt())

print()

print(t)
print(t:cube())
print(t:cbrt())
