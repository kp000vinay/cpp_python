# Test edge cases for augmented assignment

# Multiple augmented assignments in sequence
x += 1
x += 2
x += 3

# Different operators on same variable
value += 10
value -= 5
value *= 2
value //= 3

# Augmented assignment with negative numbers
x += -5
y -= -10
z *= -2

# Augmented assignment with floats
a += 3.14
b *= 0.5

# Augmented assignment with strings (concatenation)
text += "hello"
text += "world"

# Augmented assignment in nested structures
if True:
    x += 1
    if True:
        y += 2

# Augmented assignment in loops
while condition:
    counter += 1

for i in range(10):
    total += i

# Augmented assignment with parentheses
result += (x + y)
value *= (a * b)

# Augmented assignment with unary operators
x += +5
y += -value
z *= +factor

# Long identifier names
very_long_variable_name += 1
another_very_long_name_for_testing_purposes //= 2
