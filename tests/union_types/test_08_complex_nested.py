# Test 08: Complex Nested Union Types
# Category: Nested Union Types
# Priority: P1 High

# Dict of lists with unions
data: dict[str, list[int | str]] = {"nums": [1, 2], "words": ["a", "b"]}

# List of dicts with unions
records: list[dict[str, int | str | None]] = [{"id": 1, "name": "Alice"}]

# Triple nesting
complex: dict[str, list[dict[str, int | str | None]]] = {}

# With optional
optional_complex: dict[str, list[int | str] | None] = {"data": [1, "two"]}
