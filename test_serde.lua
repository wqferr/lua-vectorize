local vec = require "vectorize"

local a = vec.linspace(-100, 100, 1000)
a:save("test.luavec")
print(vec.load("test.luavec"))
