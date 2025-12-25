# Test 07: Dict with Union Types
# Category: Nested Union Types
# Priority: P1 High

# Dict with union value types
scores: dict[str, int | float] = {"alice": 95, "bob": 87.5}
config: dict[str, str | int | bool] = {"host": "localhost", "port": 8000, "debug": True}

# Dict with union key types (rare but valid)
data: dict[str | int, str] = {1: "one", "two": "2"}

# Both key and value unions
mixed: dict[str | int, int | str | None] = {"a": 1, 2: "two", "c": None}
