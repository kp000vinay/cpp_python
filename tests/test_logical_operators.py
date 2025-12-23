# Test logical operators: and, or, not

# Test 'and' operator
if x > 5 and y > 10:
    print("Both conditions true")

if a == 1 and b == 2 and c == 3:
    print("All three true")

# Test 'or' operator
if x > 5 or y > 100:
    print("At least one true")

if a == 1 or b == 2 or c == 3:
    print("At least one matches")

# Test 'not' operator
if not x == 0:
    print("Not zero")

if not (x > 10 and y < 5):
    print("Not both conditions")

# Test combined logical operators
if (x > 5 and y > 10) or (z < 0):
    print("Complex condition")

if not (x == 0 or y == 0):
    print("Neither is zero")


