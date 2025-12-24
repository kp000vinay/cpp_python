# Test 4: Walrus in list comprehension filter
result = [y for x in data if (y := transform(x)) > 0]
