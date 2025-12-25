# Test 05: Function Return Types with Unions
# Category: Basic Union Types
# Priority: P0 Critical

# Return union type
def get_value() -> int | str:
    return 5

# Return optional type
def find_user() -> str | None:
    return None

# Complex return union
def process_data() -> int | str | float | None:
    return 3.14
