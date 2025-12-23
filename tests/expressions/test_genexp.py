# Test generator expressions

# Simple generator expression
gen = (x * x for x in range(10))

# Generator expression with condition
even_gen = (x for x in range(20) if x % 2 == 0)

# Generator expression in function call
total = sum(x * x for x in range(10))











