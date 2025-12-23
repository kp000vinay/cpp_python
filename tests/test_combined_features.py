# Test combined features

# Logical and comparison
if x > 5 and y < 10:
    print("Range check")

# Bitwise in condition
if (x & 1) == 0:
    print("Even number")

# List in for loop
for item in [1, 2, 3, 4, 5]:
    if item > 3:
        print(item)

# Dict access in while
i = 0
while i < len(my_dict):
    key = list(my_dict.keys())[i]
    print(my_dict[key])
    i = i + 1

# Nested structures
matrix = [[1, 2], [3, 4]]
for row in matrix:
    for val in row:
        if val > 2:
            print(val)

# Complex expression
# Note: Ternary operator "x if condition else y" is not yet supported
# result = (x + y) * (z - w) if x > 0 else 0
result = (x + y) * (z - w)

# Multiple operators
value = x | y & z ^ w
condition = not (x > 5 and y < 10) or z == 0

