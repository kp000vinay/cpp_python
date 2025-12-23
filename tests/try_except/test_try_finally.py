# Test try/finally
try:
    x = 10
    y = 20
finally:
    print("Finally block executed")
    result = x + y

# Test try/except/finally
try:
    x = 1
except ZeroDivisionError:
    print("Caught division by zero")
finally:
    print("Cleanup code")

