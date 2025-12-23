# Test yield expressions

# Generator function with yield
def gen():
    yield 1
    yield 2
    yield 3

# Generator function with yield from
def gen2():
    yield from gen()

# Yield with value
def gen3():
    x = yield 42
    return x













