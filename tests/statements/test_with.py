# Test with statement (context managers)

# Simple with statement
with open("file.txt") as f:
    pass

# Multiple context managers
with open("file1.txt") as f1, open("file2.txt") as f2:
    pass

# With statement with body
with open("file.txt") as f:
    data = f.read()
    print(data)













