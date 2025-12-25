# Test 13: Optional Type Patterns
# Category: Real-World Patterns
# Priority: P1 High

# Common optional patterns (X | None)
name: str | None = None
age: int | None = None
score: float | None = None
active: bool | None = None

# Optional in functions
def find(key: str) -> str | None:
    return None

def get_config(name: str) -> dict[str, str] | None:
    return None

# Optional complex types
data: list[int] | None = None
config: dict[str, int | str] | None = None
nested: dict[str, list[int | str] | None] | None = None
