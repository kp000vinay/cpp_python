# Test attribute type annotations (e.g., typing.Optional)
# Tests: Attribute access in type annotations
# NOTE: This test may fail if attribute access not supported in type context

def process(value: str) -> list:
    return []

x: dict = {}
y: int = 5
