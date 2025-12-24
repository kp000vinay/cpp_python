# Test augmented assignment with complex targets

# Simple variable
simple += 1

# Subscript - single
array[0] += 10

# Subscript - expression index
data[i + 1] *= 2

# Subscript - nested
matrix[row][col] //= divisor

# Attribute
obj.value -= 5

# Attribute chain
obj.nested.value += 10

# Subscript of attribute
obj.data[key] *= 2

# Attribute of subscript
array[0].value += 1

# Complex combination
obj.matrix[i][j].value **= 2
