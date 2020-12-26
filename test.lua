local vec = require "vectorize"

local t = vec.linspace(0, math.pi, 1000)
local s = t:sin()

print(("%.2f"):format(s:trapz(t)))
