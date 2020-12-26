local vec = require "vectorize"

local v = vec{1, 2, 3}
local w = vec{-1, 0, 1}
local out = vec(3)
v:add_into(3)
print(v)
v:add_into(5)
print(v)
v:add_into(w)
print(v)

print()
print(v)
print(out)
print()
v:add_into(3, out)
print(v)
print(out)
print()


out = vec(3)
print(v)
print(w)
print(out)
print()
v:add_into(w, out)
print(v)
print(w)
print(out)
