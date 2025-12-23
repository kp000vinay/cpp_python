# Comprehensive tests for list comprehensions

# Simple
squares = [x * x for x in range(10)]

# With condition
evens = [x for x in range(20) if x % 2 == 0]
odds = [x for x in range(20) if x % 2 == 1]

# Multiple conditions
filtered = [x for x in range(100) if x % 2 == 0 if x % 3 == 0]

# Nested comprehensions
matrix = [[i * j for j in range(3)] for i in range(3)]
nested = [[x + y for y in range(3)] for x in range(2)]

# With expressions
doubles = [x * 2 for x in range(10)]
strings = [str(x) for x in range(5)]

# Complex expressions
complex_list = [x * y + z for x in range(3) for y in range(2) for z in range(1)]

# With function calls
lengths = [len(str(x)) for x in range(100)]

# Multiple for clauses
pairs = [(x, y) for x in range(3) for y in range(3)]








