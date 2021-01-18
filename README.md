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
local vec = require "vec"
local v = vec {2, 0, 1}

-- Functions can be accessed by the vec table
print(vec.sum(v)) --> 3.0
-- Or directly as methods
print(v:sum()) --> 3.0

-- Operator broadcasting
print(v + 2) --> [4.0, 2.0, 3.0]
print(v ^ 0.5) --> [1.41, 0.0, 1.0]

-- 50 equally-spaced values between 0 and pi
local t = vec.linspace(0, math.pi, 50)
print(t[1], t[50]) -- 0.0   3.14

-- Evaluate sin at every point of t
local s = vec.sin(t) -- or local s = t:sin()
local area = vec.trapz(s, t) -- integrate using trapezoid rule
print(area) -- 1.99

-- Allows for in-place operations to avoid unnecessary allocs
local x = vec.linspace(-2, 2, 1001)
for _ = 1, 1000 do
    x:sin_()
    --   ^ Extra _ at the end of the function name stores
    --     result in the same variable, or you can define
    --     a destination vector as an additional parameter.
    --     See doc/vec.md for more information
end
print(x) -- sin(sin(sin(sin(...sin(x)))))
```

## Documentation

The full reference to the functions implemented in this library can be found in
the `doc` directory. See [doc/vec.md](doc/vec.md) for basic vector
functions. See also [doc/ode.md](doc/ode.md) for basic ODE numerical
integration functions.

## Future Development

For now I am the only developer for the project. The current things I wish to
implement in the future can be found in the [TODO.md](TODO.md) file.
