# Test yield from in a loop
def flatten(items):
    for item in items:
        yield from item
