local vec = require "vectorize"

local a = vec {1, -3, 2}
local a_ = a:normalize()
print(a_, a_:norm2())
