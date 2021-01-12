# `vectorize.ode` module functions

Custom types used in the annotations below are defined in the [type
definitions section](#type-definitions).

## Built-in solvers

All functions described in this section return a [`solver`](#solver) object.
If a `number` is given to any as the initial state `x0`, it is converted to a
single-elment `vector`. The parameter `h` determines the time step size of the
method.

### `ode.euler(f: diff_f, x0: vector | number, h: number): solver`

This method evaluates `f` **once** per iteration step.

Integrate `f` using [Euler's
method](https://en.wikipedia.org/wiki/Euler_method). This is typically much
less precise, but for simple simulations it could suffice.

### `ode.heun(f: diff_f, x0: vector | number, h: number): solver`

This method evaluates `f` **twice** per iteration step.

Integrate `f` using [Heun's
method](https://en.wikipedia.org/wiki/Heun%27s_method). This is a decent
compromise between accuracy and efficiency. Sits somewhere in the middle of
Euler's method and `RK4`.

### `ode.rk4(f: diff_f, x0: vector | number, h: number): solver`

This method efaluates f **4 times** per iteration step.

Integrate `f` using an especially common 4th order [Runge-Kutta
method](https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods) known as
"RK4". Due to the higher number of evaluations of `f` per iteration step, RK4
tends to be much slower than the other built-in methods. Despite that, it is a
very popular choice due to its accuracy.

### `ode.custom`

#### TODO

## Type definitions

### `vector`

Userdata representing a value in $R^n$. Can be created from a list like in the following snippet:

```lua
local vec = require "vectorize"
local my_v = vec.from {1, 2, 3} -- or simply vec {1, 2, 3}
print(my_v) --> [1.0, 2.0, 3.0]
```

See [vector.md](vector.md) for the full documentation.

### `diff_f`

`diff_f: function(t: number, x: vector): vector | nil`

Function to be integrated. Receives the current time `t` of the simulation,
along with its current state `x`. It may return the new state, or it may alter
`x` in-place. If `nil` is returned, the latter is assumed.

### `solver`

#### TODO
