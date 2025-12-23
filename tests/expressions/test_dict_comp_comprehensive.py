# Comprehensive tests for dictionary comprehensions

# Simple
squares = {x: x * x for x in range(10)}
doubles = {x: x * 2 for x in range(5)}

# With condition
even_squares = {x: x * x for x in range(20) if x % 2 == 0}
filtered = {k: v for k, v in items if k > 0}

# Multiple conditions
filtered2 = {x: x * 2 for x in range(100) if x % 2 == 0 if x % 3 == 0}

# Complex keys and values
complex_dict = {x * 2: x * x for x in range(10)}
string_dict = {str(x): x * 2 for x in range(5)}

# Nested (from another dict)
new_dict = {k: v * 2 for k, v in old_dict.items()}

# With function calls
length_dict = {x: len(str(x)) for x in range(20)}








