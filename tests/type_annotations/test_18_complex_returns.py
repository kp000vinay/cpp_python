# Test complex return type annotations
# Tests: Subscript types in returns

def get_items() -> list[int]:
    return [1, 2, 3]

def get_mapping() -> dict[str]:
    return {}

def get_pairs() -> list[tuple]:
    return []

def get_nested() -> dict[list]:
    return {}
