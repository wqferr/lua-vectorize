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
