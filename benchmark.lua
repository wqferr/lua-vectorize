local vec = require "vectorize"

-- BENCHMARK CONFIG

local v = vec.linspace(-2, 2, 2001)

local n = 10000
local f1 = vec.square
local args1 = {}
local f2 = vec.pow
local args2 = {2}
local fraw = function(x)
    return x * x
end
local argsraw = {}


-- BENCHMARK PER SE

local socket = require "socket" -- just for timing

local t1 = socket.gettime()
for _ = 1, n do
    f1(v, table.unpack(args1))
end
t1 = socket.gettime() - t1

local t2 = socket.gettime()
for _ = 1, n do
    f2(v, table.unpack(args2))
end
t2 = socket.gettime() - t2

vraw = {}
for i, x in v:iter() do
    vraw[i] = x
end

local traw = socket.gettime()
for _ = 1, n do
    local newvraw = {}
    for i, x in ipairs(vraw) do
        newvraw[i] = fraw(x, table.unpack(argsraw))
    end
end
traw = socket.gettime() - traw

print(n, "iterations")
print('\t\ttotal time\tavg time')
print(("f1 time:\t%.2f\t\t%.6f"):format(t1, t1 / n))
print(("f2 time:\t%.2f\t\t%.6f"):format(t2, t2 / n))
print(("raw time:\t%.2f\t\t%.6f"):format(traw, traw / n))
