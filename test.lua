vec = require "vectorize"

a = vec {1, 2, 3, -4}
print(math.sqrt(a:square():sum()) - a:norm())
print(a:square():sum() - a:norm2())
