local vec = require "vectorize"

local a = vec {1, -3, 2}:normalize_()
print(a, a:norm2())
