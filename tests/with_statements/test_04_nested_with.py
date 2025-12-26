# Test nested with statements
with open("outer.txt") as outer:
    with open("inner.txt") as inner:
        data = outer.read() + inner.read()
