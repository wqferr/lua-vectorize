# Vectorize

A modern numeric library for Lua.

## Motivation

While Lua is a simple, flexible, and easily expandable language, it is not
efficient when operating on sequences. In particular, this is important
when dealing with high-dimensional vectors, since even O(n) algorithms can
lead to significant wait times for large enough vectors.

Historically, [NumLua](https://github.com/carvalho/numlua) has served to
help with this use case. However, the last contribution to its GitHub
repository was 10 years ago. Newer versions of Lua have been released, and
their FFI has changed significantly, now being incompatible with the one
NumLua was coded for.

This project aims to add many features of NumLua over time. For now, it
only supports basic vector operations such as vector addition, scaling,
hadamard product, and normalizing, as well as element-wise trigonometric
and exponential functions.

## Example Usage

```lua
local vec = require "vectorize"
local v = vec {2, 0, 1}

-- Functions can be accessed by the vec table
print(vec.sum(v)) --> 3
-- Or directly as methods
print(v:sum()) --> 3

-- Operator broadcasting
print(v + 2) --> [4, 2, 3]
print(v ^ 0.5) --> [1.41, 0, 1]

-- 50 equally-spaced values between 0 and pi
local t = vec.linspace(0, math.pi, 50)
-- Evaluate sin at every point of t
local s = vec.sin(t) -- or local s = t:sin()
local area = vec.trapz(s, t) -- integrate using trapezoid rule
print(area) -- 2.0
```

## Function Reference

The full reference to the functions implemented in this library can be found in the [FunctionReference.md](FunctionReference.md) file
