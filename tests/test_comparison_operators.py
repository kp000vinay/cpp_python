# Test comparison operators: ==, !=, <, >, <=, >=, is, is not, in, not in

# Test equality
if x == y:
    print("Equal")

if x != y:
    print("Not equal")

# Test ordering
if x < y:
    print("Less than")

if x > y:
    print("Greater than")

if x <= y:
    print("Less than or equal")

if x >= y:
    print("Greater than or equal")

# Test 'is' operator
if x is None:
    print("Is None")

if x is not None:
    print("Is not None")

# Test 'in' operator
if item in my_list:
    print("In list")

if key in my_dict:
    print("In dict")

if item not in my_list:
    print("Not in list")

# Test combined comparisons
# Note: Chained comparisons like "0 < x < 10" are not yet supported
# Use explicit and: if 0 < x and x < 10:
if 0 < x and x < 10:
    print("Between 0 and 10")

# Note: Multiple equality checks like "x == y == z" are not yet supported
# Use explicit and: if x == y and y == z:
if x == y and y == z:
    print("All equal")

