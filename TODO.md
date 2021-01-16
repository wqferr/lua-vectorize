# To do

- [ ] Benchmark tables comparing raw lua implementations vs `vectorize` implementations
- [ ] `vectorize/compat.h` to clean up vectorize.c
- [ ] More unit tests
- [ ] vec functions
    - [ ] vec:diff([n])
    - [x] vec:neg() without using vec:scale()
    - [ ] vec:expm1() -> exp(x) - 1
    - [ ] vec:abs(), vec:nabs()
    - [ ] vec:minmax()
    - [ ] vec.lerp(t, from, to[, left[, right]])
- [ ] Possible future features:
    - [ ] Matrix operations
    - [ ] Complex numbers
    - [ ] Index-sequence accessing
    - [ ] Sequences/lists which can be appended to
    - [ ] FFT
    - [ ] Polynomials
    - [ ] Statistics
