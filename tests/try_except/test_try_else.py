# Test try/except/else
try:
    x = 1
    y = 2
except ValueError:
    print("Error occurred")
else:
    print("No exception occurred")
    result = x + y

# Test try/except/else with multiple except blocks
try:
    value = 42
except ValueError:
    print("ValueError")
except TypeError:
    print("TypeError")
else:
    print("Success")
    processed = value * 2

