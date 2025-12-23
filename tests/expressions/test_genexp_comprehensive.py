# Comprehensive tests for generator expressions

# Simple
gen = (x * x for x in range(10))
gen2 = (x for x in range(20))

# With condition
even_gen = (x for x in range(20) if x % 2 == 0)
filtered = (x * 2 for x in range(100) if x > 50)

# Multiple conditions
filtered2 = (x for x in range(100) if x % 2 == 0 if x % 3 == 0)

# In function calls (without parentheses)
total = sum(x * x for x in range(10))
max_val = max(x * 2 for x in range(20) if x > 10)
min_val = min(x for x in range(100) if x % 7 == 0)

# Multiple arguments with genexp
result = sum(x for x in range(10)) + sum(y for y in range(5))

# Nested generator expressions
nested = ((x, y) for x in range(3) for y in range(3))

# Complex expressions
complex_gen = (x * y + z for x in range(3) for y in range(2) for z in range(1))

# With function calls in expression
lengths = (len(str(x)) for x in range(100))








