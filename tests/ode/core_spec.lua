require "luarocks.require"
local ode = require "vec.ode"

describe(
  "ode",
  function()
    describe(
      "integration methods",
      function()
        for _, method in ipairs {"euler", "heun", "rk4"} do
          it(
            ("should include %q"):format(method),
            function()
              assert.is_function(ode[method])
            end
          )
        end
      end
    )
  end
)
