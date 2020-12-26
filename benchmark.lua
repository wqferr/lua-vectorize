-- BENCHMARK CONFIG

local vec = require "vectorize"
local v = vec.linspace(-2, 2, 2001)

local n = 100000
local f1 = function(x)
    return x:ln1p()
end
local args1 = {}

local f2 = function(x)
    return (x + 1):ln()
end
local args2 = {}

local fraw = function(x)
    return math.log(1 + x)
end
local argsraw = {}

-- BENCHMARK PER SE

pcall(require, "luarocks.require")
local socket = require "socket" -- just for timing

local t1, t2, traw
if f1 then
    t1 = socket.gettime()
    for _ = 1, n do
        f1(v, table.unpack(args1))
    end
    t1 = socket.gettime() - t1
end

if f2 then
    t2 = socket.gettime()
    for _ = 1, n do
        f2(v, table.unpack(args2))
    end
    t2 = socket.gettime() - t2
end

if fraw then
    vraw = {}
    -- build raw vec in lua
    for i, x in v:iter() do
        vraw[i] = x
    end

    traw = socket.gettime()
    for _ = 1, n do
        local newvraw = {}
        for i, x in ipairs(vraw) do
            newvraw[i] = fraw(x, table.unpack(argsraw))
        end
    end
    traw = socket.gettime() - traw
end

print(("%d iterations"):format(n))
print()
print("function\ttotal time\tavg time")
if t1 then
    print(("f1 time:\t%10.2f\t%8.6f"):format(t1, t1 / n))
end
if t2 then
    print(("f2 time:\t%10.2f\t%8.6f"):format(t2, t2 / n))
end
if traw then
    print(("raw time:\t%10.2f\t%8.6f"):format(traw, traw / n))
end
