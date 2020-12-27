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

Functions marked with a `(I)` have an in-place variant.

### TODO explain in-place variants for functions somewhere

### Constructors

---

#### `vec.new(size: number): vector`

Create a new vector with the given length.  
`size` must be a positive integer; all elements will be `0.0`.

This function can also be called as `vec(size)`.

---

#### `vec.ones(size: number): vector`

Create a new vector wiht the given length.  
`size` must be a positive integer; all elements will be `1.0`.

---

#### `vec.from(list: {number}): vector`

Create a new vector from the given elements.  
All elements of the given list must be numbers.

This function can also be called as `vec(list)`.

---

#### `vec.basis(size: number, b: number): vector`

Create a new vector where all elements are `0.0`, except for `v[b]`, which
will be `1.0`.  
`size` and `b` must be positive integers.

---

#### `vec.linspace(start: number, end: number, n: number): vector`

Create a new vector with length `n` whose elements are equally spaced between
`start` and `end` (inclusive).  
`n` must be a positive integer.

---

#### `vec.dup(original: vector): vector (I)`

Create a new vector with the same length and elements as `original`.

---

### Serialization / Deserialization

#### `vec.save(v: vector, filename: string)`

Save the vector data to a file.  
The recommended extension for the file is `*.luavec`.

Please note that transferring a vector file from one machine to another is
not guaranteed to work. For more information, see `vec.load`.

---

#### `vec.load(filename: string): vector`

Load a vector from a file generated by `vec.save`.

Please note that transferring a vector file from one machine to another is
not guaranteed to work. If the machine architectures differ in the number
of bytes of `lua_Integer` or `lua_Number`, `vec.load` will detect this and
generate an error.

---

### Arithmetic

---

#### `vec.add(x, y): vector (I)`

This function is called automatically by `x + y`. Errors if the two vectors
don't have the same length.

---

##### `x: vector, y: vector`

Element-wise addition of `x` and `y`.

---

##### `x: vector, y: number`

Add `y` to every element of `x`.

---

##### `x: number, y: vector`

Add `x` to every element of `y`.

---

#### `vec.sub(x, y): vector (I)`

This function is called automatically by `x - y`.

---

##### `x: vector, y: vector`

Element-wise subtraction of `x` and `y`. Errors if the two vectors don't
have the same length.

---

##### `x: vector, y: number`

Subtract `y` from every element of `x`.

---

##### `x: number, y: vector`

Subtract every element of `y` from `x`.

---

#### `vec.mul(x, y): vector (I)`

This function is called automatically by `x * y`.

---

##### `x: vector, y: vector`

Element-wise multiplication of `x` and `y`. Equivalent to `vec.hadamard(x, y)`. Errors if the two vectors don't have the same length.

---

##### `x: vector, y: number`

Equivalent to `vec.scale(x, y)`.

---

##### `x: number, y: vector`

Equivalent to `vec.scale(y, x)`.

---

#### `vec.div(x, y): vector (I)`

This function is called automatically by `x / y`.

---

##### `x: vector, y: vector`

Element-wise division of `x` and `y`. Errors if the two vectors don't have
the same length. Roughly equivalent to `x:hadamard(y:reciproc())`. Using this
function instead of the previous snippet will tend to yield slightly more
precise results.

---

##### `x: vector, y: number`

Divide every element of `x` by `y`. Roughly equivalent to `vec.scale(x, 1/y)`. Using this function instead of the previous snippet will tend to
yield slightly more precise results.

---

##### `x: number, y: vector`

Divide `y` by every element of `x`. Roughly equivalent to
`vec.scale(y:reciproc(), x)`. Using this function instead of the previous
snippet will tend to yield slightly more precise results.

---

#### `vec.pow(x, y): vector (I)`

This function is called automatically by `x ^ y`.

---

##### `x: vector, y: vector`

Element-wise exponentiation of `x` and `y`. Errors if the two vectors don't
have the same length.

---

##### `x: vector, y: number`

Raise every element of `x` to the `y`-th power.

---

##### `x: number, y: vector`

Raise `x` to the power of each element of `y`.

---

#### `vec.neg(x: vector): vector (I)`

This function is called automatically by `-x`.

Element-wise negation of `x`. Optimized equivalent to `x:scale(-1)`.

---

#### `vec.reciproc(x: vector): vector (I)`

Element-wise reciprocal of `x`. More precise equivalent to `vec.div(1, x)`.

---

#### `vec.psy(x: vector, s: number, y: vector): vector (I)`

"Plus scaled y": adds `x` to `y` scaled by a factor of `s`. Roughly
equivalent to `x + s*y`. Using this function instead of the previous
snippet will tend to yield slightly more precise results at a slightly
better performance. Errors if the two vectors don't have the same length.

---

#### `vec.hadamard(x: vector, y: vector): vector (I)`

Element-wise product of `x` and `y`. Errors if the two vectors don't have
the same length.

---

#### `vec.scale(x: vector, s: number): vector (I)`

Element-wise multiplication of `x` by `s`.

---

#### `vec.inner(x: vector, y: vector): number`

Inner product between `x` and `y`. Also known as the scalar product between
`x` and `y`. Errors if the two vectors don't have the same length.
