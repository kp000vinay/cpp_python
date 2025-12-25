# Test complex type annotations for variables
# Tests: Subscript types (list[int], dict[str])

numbers: list[int] = [1, 2, 3]
mapping: dict[str] = {"a": "b"}
pairs: list[tuple] = []
nested: dict[list] = {}
items: list[str] = ["x", "y"]
values: list[float] = [1.0, 2.0]
