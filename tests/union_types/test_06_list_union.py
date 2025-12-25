# Test 06: List with Union Element Types
# Category: Nested Union Types
# Priority: P1 High

# List of union types
items: list[int | str] = [1, "two", 3]
values: list[int | str | float] = [1, "2", 3.0]
data: list[str | None] = ["a", None, "c"]

# Nested lists with unions
matrix: list[list[int | float]] = [[1, 2.5], [3.0, 4]]
