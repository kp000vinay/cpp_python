# Test mixed parameter annotations
# Tests: Some parameters annotated, others not

def func(a: int, b, c: str, d):
    pass

def process(x, y: float, z):
    pass

def calculate(a: int, b: int, c):
    return a + b
