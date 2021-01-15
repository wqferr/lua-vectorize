# `vec.ode` module functions

Custom types used in the annotations below are defined in the [type
definitions section](#type-definitions).

## Built-in solvers

All functions described in this section return a [`solver`](#solver) object.
If a `number` is given to any as the initial state `x0`, it is converted to a
single-elment `vector`. The parameter `h` determines the time step size of the
method.

All build-in integration methods use in-place operations, which means the
reference to their states remains the same across iterations. If you need
to store a state for later, you need to use `saved = solver.state:dup()`
to preserve `saved` despite `solver.state` changing.

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

## Type definitions

### `vector`

Userdata representing a value in $R^n$. Can be created from a list like in the following snippet:

```lua
local vec = require "vec"
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

#### Public fields

- `integrand`: function being integrated;
- `t`: current simulation time;
- `iter`: number of steps performed;
- `state`: current simulation state;

#### Methods

- `solver:step(): number, vector`  
  Perform a simulation step according to the integration method used to
  create the solver.
- `solver:multistep(n: number | nil): function`  
  To be used in `for` loops, this calls `solver:step` repeatedly. If `n` is
  ommited, it will never break the loop on its own.

#### Examples

##### Going around in a circle

TODO

##### Using `iterstep`

```lua
local vec = require "vec"
local ode = require "vec.ode"

local function integrand(_t, x)
  x[1], x[2] = -x[2], x[1]
end

local history = {}

local s = ode.rk4(integrand, vec {1, 0}, 0.05)
for t, state in s:multistep(100) do -- Perform 100 iterations
  table.insert(history, state:dup()) -- Clone state so we can use it later!
  print(t, state, state:norm())
end

-- At this point history would still have valid references
do_stuff(history)
```
