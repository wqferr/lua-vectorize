local vec = require "vectorize"
local M = {}
local solver = {}
local solver_mt = {__index = solver}

local function custom_solver(intmethod_step, integrand, initstate, cfg)
  local s =
    setmetatable(
    {
      integrand = integrand,
      intmethod_step = intmethod_step,
      t = 0,
      iter = 0,
      state = initstate,
      cfg = cfg
    },
    solver_mt
  )
  return s
end
M.custom_solver = custom_solver

function solver:step()
  self.iter = self.iter + 1
  self.t, self.state =
    self.intmethod_step(self.integrand, self.t, self.state, self.cfg, self.iter)
  return self.t, self.state
end

local function euler_step(f, t, state, cfg, iter)
  local buf = cfg.buffer
  state:dup_(buf)
  local deriv = f(t, buf) or buf
  state:add_(deriv:scale_(cfg.stepsize))

  return iter * cfg.stepsize, state
end

local function euler(integrand, initstate, stepsize)
  if type(initstate) == "number" then
    initstate = vec {initstate}
  end
  local cfg = {
    stepsize = stepsize,
    buffer = vec(#initstate)
  }
  return custom_solver(euler_step, integrand, initstate, cfg)
end
M.euler = euler

return M
