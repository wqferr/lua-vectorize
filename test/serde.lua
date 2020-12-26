local vec = require "vectorize"
local fname = "test/a.luavec"

local a = vec.linspace(-100, 100, 1000)
a:save(fname)
print(vec.load(fname))
