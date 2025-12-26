# Test yield in a loop
def counter(n):
    i = 0
    while i < n:
        yield i
        i = i + 1
