# Test slicing

# Simple slices
arr = [1, 2, 3, 4, 5]
s1 = arr[1:3]
s2 = arr[:3]
s3 = arr[1:]
s4 = arr[:]

# Slices with step
s5 = arr[::2]
s6 = arr[1:4:2]
s7 = arr[::-1]

# Complex slices
s8 = arr[start:end]
s9 = arr[start:end:step]

# Nested slicing
s10 = arr[1:3][0:1]








