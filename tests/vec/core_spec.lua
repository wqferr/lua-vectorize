require "luarocks.require"
local vec = require "vectorize"

describe(
  "constructor",
  function()
    it(
      "should create a vector with the specified length",
      function()
        assert.are.equal(3, #vec(3))
        assert.are.equal(5, #vec(5))
      end
    )
    it(
      "should error with non-positive size",
      function()
        assert.has.errors(
          function()
            vec(-1)
          end
        )
        assert.has.errors(
          function()
            vec(0)
          end
        )
      end
    )
    it(
      "should create vectors whose entries are 0",
      function()
        local v = vec(10)
        for _, x in v:iter() do
          assert.are.equal(0, x)
        end
      end
    )
  end
)
describe(
  "read-indexing",
  function()
    it(
      "should return the correct element",
      function()
        local v = vec {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
        for i = 1, 10 do
          assert.are.equal(i, v[i])
        end
      end
    )
    it(
      "should error on oob-accessing",
      function()
        local v = vec(3)
        assert.has.errors(
          function()
            _ = v[0]
          end
        )
        assert.has.errors(
          function()
            _ = v[4]
          end
        )
        assert.has.errors(
          function()
            _ = v[-1]
          end
        )
      end
    )
  end
)
describe(
  "write-indexing",
  function()
    it(
      "should replace the correct element",
      function()
        local v = vec(5)
        v[3] = 1

        assert.are.equal(0, v[1])
        assert.are.equal(0, v[2])
        assert.are.equal(1, v[3])
        assert.are.equal(0, v[4])
        assert.are.equal(0, v[5])
      end
    )
    it(
      "should error on oob-accessing",
      function()
        local v = vec(3)
        assert.has.errors(
          function()
            v[0] = 1
          end
        )
        assert.has.errors(
          function()
            v[4] = 1
          end
        )
        assert.has.errors(
          function()
            v[-1] = 1
          end
        )
      end
    )
  end
)
