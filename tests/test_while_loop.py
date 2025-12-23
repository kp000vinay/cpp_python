# Test while loops

# Simple while loop
i = 0
while i < 10:
    print(i)
    i = i + 1

# While with condition
x = 0
while x < 100:
    x = x + 10
    print(x)

# While with else clause
count = 0
while count < 5:
    print(count)
    count = count + 1
else:
    print("Loop completed")

# Nested while loops
i = 0
while i < 3:
    j = 0
    while j < 3:
        print(i, j)
        j = j + 1
    i = i + 1

# While with break condition
x = 0
while True:
    if x > 10:
        break
    x = x + 1


