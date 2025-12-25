# Test 09: Parenthesized Union Types
# Category: Edge Cases
# Priority: P2 Medium

# Explicit parentheses (should work same as without)
x: (int | str) = 5
y: (int | str) | float = 3.14
z: int | (str | float) = "hello"

# Complex parenthesization
w: ((int | str) | float) | None = None
v: (int | (str | (float | None))) = 5
