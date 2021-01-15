local vec = require "vectorize"
local M = {}
local solver = {}
local solver_mt = {__index = solver}

local function _solver(intmethod_step, integrand, initstate, cfg)
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

-- TODO expose custom solver creation in lib

function solver:step()
  self.iter = self.iter + 1
  self.t, self.state =
    self.intmethod_step(self.integrand, self.t, self.state, self.cfg, self.iter)
  return self.t, self.state
end

function solver:multistep(n)
  if n == nil then
    n = math.huge
  end

  return function()
    if n > 0 then
      n = n - 1
      return self:step()
    else
      return nil
    end
  end
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
  return _solver(euler_step, integrand, initstate, cfg)
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
  if type(initstate) == "number" then
    initstate = vec {initstate}
  end
  local cfg = {
    stepsize = stepsize,
    buf = {
      vec(#initstate),
      vec(#initstate)
    }
  }
  return _solver(heun_step, integrand, initstate, cfg)
end
M.heun = heun

local function rk4_step(f, t, state, cfg, iter)
  local buf = cfg.buf
  local h = cfg.stepsize
  local h_2 = h / 2

  state:dup_(buf[1])
  local k1 = f(t, buf[1]) or buf[1]

  local t_interm = t + h / 2

  buf[1]:scale_(h_2, buf[2])
  local k2_state = buf[2]:add_(state)
  local k2 = f(t_interm, k2_state) or k2_state

  buf[2]:scale_(h_2, buf[3])
  local k3_state = buf[3]:add_(state)
  local k3 = f(t_interm, k3_state) or k3_state

  local t_final = t + h
  buf[3]:scale_(h, buf[4])
  local k4_state = buf[4]:add_(state)
  local k4 = f(t_final, k4_state) or k4_state

  k2:add_(k3)
  k1:add_(k4)
  k1:psy_(2, k2)
  state:psy_(h / 6, k1)

  return iter * h, state
end

local function rk4(integrand, initstate, stepsize)
  if type(initstate) == "number" then
    initstate = vec {initstate}
  end
  local cfg = {
    stepsize = stepsize,
    buf = {
      vec(#initstate),
      vec(#initstate),
      vec(#initstate),
      vec(#initstate)
    }
  }
  return _solver(rk4_step, integrand, initstate, cfg)
end
M.rk4 = rk4

return M
