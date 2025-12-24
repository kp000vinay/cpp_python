# Test 16: Walrus in dict comprehension
result = {k: v for k in keys if (v := get_value(k)) is not None}
