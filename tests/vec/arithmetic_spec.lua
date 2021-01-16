pcall(require, "luarocks.require")
local vec = require "vec"

local function test_binary_op(op)
  it(
    "can be used element-wise",
    function()
      local u = vec {3, 2, 1}
      local v = vec {4, -1, 0}
      local w = op(u, v)
      local w_ = op(v, u)

      assert.are.equal(#u, #w)
      assert.are.equal(#u, #w_)

      for i = 1, #w do
        assert.are.equal(op(u[i], v[i]), w[i])
        assert.are.equal(op(v[i], u[i]), w_[i])
      end
    end
  )
  it(
    "can broadcast a right-scalar",
    function()
      local u = vec {1, 2, 3}
      local s = 4
      local v = op(u, s)

      assert.are.equal(#u, #v)

      for i = 1, #u do
        assert.are.equal(op(u[i], s), v[i])
      end
    end
  )
  it(
    "can broadcast a left-scalar",
    function()
      local u = vec {1, 2, 3}
      local s = 4
      local v = op(s, u)

      assert.are.equal(#u, #v)

      for i = 1, #u do
        assert.are.equal(op(s, u[i]), v[i])
      end
    end
  )
  it(
    "errors when used in vectors of different shapes",
    function()
      local u = vec(2)
      local v = vec(3)
      assert.has_error(
        function()
          op(u, v)
        end
      )
    end
  )
end

describe(
  "operator",
  function()
    it(
      "unary minus",
      function()
        local v = vec {1, -2, 3, -4, 0, -0}
        local comp = -v
        for i, original in v:iter() do
          assert.are.equal(-original, comp[i])
        end
      end
    )
    it(
      "len",
      function()
        local u = vec(7)
        local v = vec {1, 2, 3}

        assert.are.equal(7, #u)
        assert.are.equal(3, #v)
      end
    )
    describe(
      "add",
      function()
        test_binary_op(
          function(a, b)
            return a + b
          end
        )
      end
    )
    describe(
      "sub",
      function()
        test_binary_op(
          function(a, b)
            return a - b
          end
        )
      end
    )
    describe(
      "mul",
      function()
        test_binary_op(
          function(a, b)
            return a * b
          end
        )
      end
    )
    describe(
      "div",
      function()
        test_binary_op(
          function(a, b)
            return a / b
          end
        )
      end
    )
    describe(
      "pow",
      function()
        test_binary_op(
          function(a, b)
            return a ^ b
          end
        )
      end
    )
  end
)
