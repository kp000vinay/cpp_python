# Test augmented assignment with complex right-hand side expressions

# Arithmetic operations
x += 2 + 3
y -= a * b
z *= (p + q) / 2

# Function calls
result += calculate(x, y)
total -= get_value()

# Nested operations
value += 2 * (3 + func(a, b))

# Comparisons (result in boolean)
count += x > y

# Boolean operations
flag |= a and b
mask &= not disabled

# List/tuple operations
data += [1, 2, 3]
items += (x, y, z)

# Dictionary/set operations (if supported)
config += {}

# Conditional expression
value += x if condition else y

# Multiple operators
result += a + b * c - d / e
