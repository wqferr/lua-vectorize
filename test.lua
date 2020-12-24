local vec = require'vectorize'
local a = vec(3)
print(pcall(function() a[5] = 0 end))
print(pcall(function() a[-1] = 0 end))
print(pcall(function() return a[-1] end))
print(pcall(function() return a[10] end))
