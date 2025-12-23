# Test mixing different comprehension types

# List from generator
list_from_gen = list(x * x for x in range(10))

# Set from generator
set_from_gen = set(x for x in range(20) if x % 2 == 0)

# Dict from generator (with tuple unpacking simulation)
# Note: This would need tuple support in targets

# Mixed comprehensions in expressions
result = sum([x * x for x in range(10)])
result2 = max({x for x in range(20) if x % 2 == 0})

# Comprehensions with conditionals
mixed = [x if x > 5 else 0 for x in range(10)]
mixed2 = {x: (x * 2 if x > 0 else 0) for x in range(10)}

# Nested with different types
nested_mixed = [[x * y for y in range(3)] for x in range(2)]








