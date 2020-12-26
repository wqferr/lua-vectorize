vec = require "vectorize"

print(
    ("%.2f"):format(
        vec.linspace(0, 2 * math.pi, 1000):sin():trapz(0, 2 * math.pi)
    )
)
