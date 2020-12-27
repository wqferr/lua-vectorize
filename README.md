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

#### `vec.add(a, b): vector (I)`

This function is called automatically by `a + b`. Errors if the two vectors
don't have the same length.

---

##### `a: vector, b: vector`

Element-wise addition of `a` and `b`.

---

##### `a: vector, b: number`

Add `b` to every element of `a`.

---

##### `a: number, b: vector`

Add `a` to every element of `b`.

---

#### `vec.sub(a, b): vector (I)`

This function is called automatically by `a - b`.

---

##### `a: vector, b: vector`

Element-wise subtraction of `a` and `b`. Errors if the two vectors don't
have the same length.

---

##### `a: vector, b: number`

Subtract `b` from every element of `a`.

---

##### `a: number, b: vector`

Subtract every element of `b` from `a`.

---

#### `vec.mul(a, b): vector (I)`

This function is called automatically by `a * b`.

---

##### `a: vector, b: vector`

Element-wise multiplication of `a` and `b`. Equivalent to `vec.hadamard(a, b)`. Errors if the two vectors don't have the same length.

---

##### `a: vector, b: number`

Equivalent to `vec.scale(a, b)`.

---

##### `a: number, b: vector`

Equivalent to `vec.scale(b, a)`.

---

#### `vec.div(a, b): vector (I)`

This function is called automatically by `a / b`.

---

##### `a: vector, b: vector`

Element-wise division of `a` and `b`. Errors if the two vectors don't have
the same length. Roughly equivalent to `a:hadamard(b:reciproc)`. Using this
function instead of the previous snippet will tend to yield slightly more
precise results.

---

##### `a: vector, b: number`

Divide every element of `a` by `b`. Roughly equivalent to `vec.scale(a, 1/b)`. Using this function instead of the previous snippet will tend to
yield slightly more precise results.

---

##### `a: number, b: vector`

Divide `b` by every element of `a`. Roughly equivalent to
`vec.scale(b:reciproc(), a)`. Using this function instead of the previous
snippet will tend to yield slightly more precise results.

---

#### `vec.pow(a, b): vector (I)`

This function is called automatically by `a ^ b`.

---

##### `a: vector, b: vector`

Element-wise exponentiation of `a` and `b`. Errors if the two vectors don't
have the same length.

---

##### `a: vector, b: number`

Raise every element of `a` to the `b`-th power.

---

##### `a: number, b: vector`

Raise `a` to the power of each element of `b`.

---

#### `vec.neg(a): vector (I)`

This function is called automatically by `-a`.

Element-wise negation of `a`. Optimized equivalent to `a:scale(-1)`.

---

#### `vec.reciproc(a): vector (I)`

Element-wise reciprocal of `a`. More precise equivalent to `vec.div(1, a)`.
