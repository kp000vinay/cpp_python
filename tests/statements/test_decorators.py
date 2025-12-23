# Test decorators on functions and classes

# Simple decorator
@decorator
def func1():
    pass

# Multiple decorators
@decorator1
@decorator2
def func2():
    pass

# Decorator with arguments
@decorator(arg1, arg2)
def func3():
    pass

# Decorator with attribute access
@module.decorator
def func4():
    pass

# Decorator on class
@class_decorator
class MyClass:
    pass

# Multiple decorators on class
@decorator1
@decorator2
class MyClass2:
    pass

# Complex decorator expression
@decorator().method()
def func5():
    pass




