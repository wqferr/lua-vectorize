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
  state:dup_(cfg.buf)
  local deriv = f(t, cfg.buf) or cfg.buf
  state:add_(deriv:scale_(cfg.stepsize))

  return iter * cfg.stepsize, state
end

local function euler(integrand, initstate, stepsize)
  if type(initstate) == "number" then
    initstate = vec {initstate}
  end
  local cfg = {
    stepsize = stepsize,
    buf = vec(#initstate)
  }
  return custom_solver(euler_step, integrand, initstate, cfg)
end
M.euler = euler

local function heun_step(f, t, state, cfg, iter)
  local buf = cfg.buf

  -- first evaluation
  state:dup_(buf[1])
  local deriv_l = f(t, buf[1]) or buf[1] -- derivative at initial point
  deriv_l:scale_(cfg.stepsize)

  -- intermediate state
  state:add_(deriv_l, buf[2])

  -- second evaluation
  local tnext = iter * cfg.stepsize
  local deriv_r = f(tnext, buf[2]) or buf[2] -- derivative at next point
  deriv_r:scale_(cfg.stepsize / 2)

  local deriv_avg = deriv_r:psy_(0.5, deriv_l)
  state:add_(deriv_avg)
  return tnext, state
end

local function heun(integrand, initstate, stepsize)
  if type(initstate) == 'number' then
    initstate = vec{initstate}
  end
  local cfg = {
    stepsize = stepsize,
    buf = {
      vec(#initstate),
      vec(#initstate)
    }
  }
  return custom_solver(heun_step, integrand, initstate, cfg)
end
M.heun = heun

return M
