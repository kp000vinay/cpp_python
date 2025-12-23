# Comprehensive tests for conditional expressions

# Simple cases
a = 1 if True else 2
b = 10 if False else 20

# With comparisons
x = 5 if x > 0 else -5
y = "positive" if n > 0 else "non-positive"

# Nested conditionals
z = 1 if x > 0 else 2 if x < 0 else 0
result = "a" if x > 0 else "b" if x < 0 else "c"

# In expressions
max_val = (a if a > b else b) + 10
min_val = (x if x < y else y) * 2

# Complex nested
value = (1 if a > 0 else 2) if b > 0 else (3 if c > 0 else 4)

# With function calls
result = func() if condition else other_func()

# With operators
result = x + 1 if x > 0 else x - 1








