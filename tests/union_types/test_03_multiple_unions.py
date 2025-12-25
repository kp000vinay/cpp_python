# Test 03: Multiple Union Types
# Category: Basic Union Types
# Priority: P0 Critical

# Three or more types in union
value: int | str | float = 5
data: int | str | float | None = "test"
result: int | str | float | bool = True

# Left-associative: (int | str) | float
mixed: int | str | float | None = 3.14
