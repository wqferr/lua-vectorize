local vec = require "vectorize"
local socket = require "socket" -- just for profiling

local v = vec.linspace(-2, 2, 2001)

local n = 1000
local f1 = vec.square
local args1 = {v}
local f2 = vec.pow
local args2 = {v, 2}

local t1 = socket.gettime()
for i = 1, n do
    f1(table.unpack(args1))
end
t1 = socket.gettime() - t1

local t2 = socket.gettime()
for i = 1, n do
    f2(table.unpack(args2))
end
t2 = socket.gettime() - t2

print(n, "iterations")
print("nop time:", baseline)
print('f1 time:', t1)
print('f2 time:', t2)
