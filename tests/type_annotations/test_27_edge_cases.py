# Test edge cases and special scenarios
# Tests: Various edge cases in annotation syntax

# Empty parameter list with return annotation
def get_value() -> int:
    return 42

# Single annotation without value at module level
result: int

# Annotation in nested function
def outer():
    def inner(x: int) -> str:
        return str(x)
    return inner

# Multiple functions with annotations
def func1(a: int) -> int:
    return a

def func2(b: str) -> str:
    return b
