# Comprehensive test for all implemented features

# ============================================
# 1. CONDITIONAL EXPRESSIONS (Ternary Operator)
# ============================================
x = 1 if True else 0
y = "yes" if x > 0 else "no"
z = 10 if False else 20
result = a if condition else b
nested = x if y > 0 else (z if w < 0 else 0)

# ============================================
# 2. LIST COMPREHENSIONS
# ============================================
# Basic list comprehension
lst1 = [x for x in range(10)]

# With condition
lst2 = [x * 2 for x in range(10) if x % 2 == 0]

# Nested
lst3 = [x * y for x in range(5) for y in range(3)]

# With multiple conditions
lst4 = [x for x in range(20) if x % 2 == 0 if x % 3 == 0]

# ============================================
# 3. DICTIONARY COMPREHENSIONS
# ============================================
# Basic dict comprehension
d1 = {k: v for k, v in enumerate(range(5))}

# With condition
d2 = {k: v * 2 for k, v in enumerate(range(10)) if v % 2 == 0}

# Key-value transformation
d3 = {str(k): k * 2 for k in range(5)}

# ============================================
# 4. SET COMPREHENSIONS
# ============================================
# Basic set comprehension
s1 = {x for x in range(10)}

# With condition
s2 = {x * 2 for x in range(10) if x % 2 == 0}

# ============================================
# 5. GENERATOR EXPRESSIONS
# ============================================
# Basic generator
gen1 = (x for x in range(10))

# With condition
gen2 = (x * 2 for x in range(10) if x % 2 == 0)

# In function call (implicit generator)
result1 = sum(x for x in range(10) if x % 2 == 0)
result2 = max(x * 2 for x in range(5))

# ============================================
# 6. SLICING
# ============================================
arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

# Basic slice
slice1 = arr[1:5]

# Start only
slice2 = arr[3:]

# End only
slice3 = arr[:5]

# Step only
slice4 = arr[::2]

# Start and step
slice5 = arr[1::2]

# End and step
slice6 = arr[:5:2]

# Full slice
slice7 = arr[1:5:2]

# Empty slice (all elements)
slice8 = arr[:]

# Negative indices
slice9 = arr[-5:-1]
slice10 = arr[::-1]

# ============================================
# 7. DECORATORS
# ============================================
# Single decorator
@decorator
def func1():
    return 1

# Multiple decorators
@decorator1
@decorator2
@decorator3
def func2():
    return 2

# Decorator with arguments
@decorator(arg1, arg2)
def func3():
    return 3

# Decorator with attribute access
@module.decorator
def func4():
    return 4

# Decorator on class
@class_decorator
class MyClass:
    pass

# Multiple decorators on class
@decorator1
@decorator2
class MyClass2:
    pass

# ============================================
# 8. ELLIPSIS
# ============================================
# Basic ellipsis
ellipsis1 = ...

# In tuple
t = (1, 2, ...)

# In list
lst_ellipsis = [1, 2, ...]

# In dict (as key)
d_ellipsis1 = {...: 1}

# In dict (as value)
d_ellipsis2 = {"key": ...}

# In function call
func(...)

# ============================================
# 9. COMBINED FEATURES
# ============================================
# Conditional in comprehension
combined1 = [x if x > 0 else 0 for x in range(-5, 5)]

# Comprehension in slice
arr2 = [x * 2 for x in range(10)]
slice_combined = arr2[1:5:2]

# Decorated function with comprehensions
@decorator
def func_with_comprehensions():
    return [x * 2 for x in range(10) if x % 2 == 0]

# Lambda with ellipsis
f = lambda x: ... if x else None

# Generator in function call with conditional
result3 = sum(x if x > 0 else 0 for x in range(-5, 5))


