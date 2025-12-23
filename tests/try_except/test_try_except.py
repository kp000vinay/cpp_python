# Test try/except statements
try:
    x = 1 / 0
except ZeroDivisionError:
    print("Caught division by zero")
    result = "error"

# Test try/except with exception type and name
try:
    value = int("not a number")
except ValueError as e:
    print("Caught ValueError")
    error_msg = "value error"

# Test multiple except blocks
try:
    data = {"key": "value"}
    value = data["missing"]
except KeyError:
    print("Key not found")
except Exception as e:
    print("Other error")

# Test bare except
try:
    x = 1
except:
    print("Caught all exceptions")

