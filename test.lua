vec = require "vectorize"

a = vec {1, 2, 3}
print(a)
print(a+a)
print(a+1)
print(1+a)
a:add_into(1)
print(a)
print(a-a)
print((a+1)-(1+a))
for i, v in a:iter() do
    print(i, v)
end
