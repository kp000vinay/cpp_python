# Test multiple context managers
with open("a.txt") as a, open("b.txt") as b:
    data = a.read()
