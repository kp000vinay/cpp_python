# Test 17: Walrus in set comprehension
result = {y for x in data if (y := process(x)) > 0}
