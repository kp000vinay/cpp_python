# Test 01: Simple Union Types
# Category: Basic Union Types
# Priority: P0 Critical

# Simple two-type union
x: int | str = 5
y: int | str = "hello"

# Verify both int and str work
z: int | str = 42
w: int | str = "world"
