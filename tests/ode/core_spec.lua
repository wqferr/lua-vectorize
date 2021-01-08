require "luarocks.require"
local ode = require "vectorize.ode"

describe(
  "ode",
  function()
    describe(
      "integration methods",
      function()
        for _, method in ipairs {"euler", "heun", "rk4", "custom"} do
          it(
            ("should include %q"):format(method),
            function()
              assert.is_function(ode[method])
            end
          )
        end
      end
    )
    describe(
      "api",
      function()
        it(
          'should have a "shortcut" route',
          function()
            assert(false)
            -- TODO check ode is callable
          end
        )
        -- TODO more tests
      end
    )
  end
)
