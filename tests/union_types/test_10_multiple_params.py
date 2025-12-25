# Test 10: Multiple Parameters with Unions
# Category: Edge Cases
# Priority: P2 Medium

# Many parameters with different union types
def process(
    a: int | str,
    b: float | None,
    c: str | int | bool,
    d: list[int | str]
) -> dict[str, int | str | None]:
    return {}

# Mix of union and non-union parameters
def transform(
    x: int,
    y: int | str,
    z: float,
    w: str | None
) -> int | float:
    return 0
