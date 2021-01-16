package = "vectorize"
version = "scm-0"
source = {
  url = "git://github.com/wqferr/lua-vectorize.git"
}

description = {
  summary = "Simple and naïve vectorization for Lua.",
  detailed = [[
Vectorized routines akin to numpy's array. Currently only supports 1D arrays, with element-wise operators and common functions such as exp, sin, project, and friends.
Aims to be a replacement for NumLua for newer versions of Lua (since the last commit on that project was 10 years ago), but this is still a very young project.
Contributions are welcome!]],
  homepage = "https://github.com/wqferr/lua-vectorize",
  license = "GPL-3.0"
}

dependencies = {
  "lua >= 5.1, < 5.5"
}

build = {
  type = "builtin",
  modules = {
    vec = {
      sources = {"vectorize.c"}
      -- this source depends on libm, but Lua is
      -- already linked with it
    },
    ["vec.ode"] = "ode.lua"
  }
}
