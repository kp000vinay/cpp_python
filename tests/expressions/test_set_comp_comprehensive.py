# Comprehensive tests for set comprehensions

# Simple
squares = {x * x for x in range(10)}
doubles = {x * 2 for x in range(5)}

# With condition
evens = {x for x in range(20) if x % 2 == 0}
filtered = {x for x in range(100) if x % 3 == 0}

# Multiple conditions
filtered2 = {x for x in range(100) if x % 2 == 0 if x % 3 == 0}

# Complex expressions
complex_set = {x * y + z for x in range(3) for y in range(2) for z in range(1)}

# With function calls
lengths = {len(str(x)) for x in range(100)}

# Multiple for clauses
pairs = {(x, y) for x in range(3) for y in range(3)}








