# Test 14: Comprehensive Union Types Test
# Category: Comprehensive
# Priority: P0 Critical

# All union type patterns in one file

# Basic unions
x: int | str = 5
y: str | None = None
z: int | str | float = 3.14

# Function signatures
def process(value: int | str) -> str | None:
    return None

async def fetch(id: int | None) -> dict[str, int | str | None]:
    return {}

# Nested in collections
items: list[int | str] = [1, "two"]
scores: dict[str, int | float] = {"alice": 95, "bob": 87.5}
complex: dict[str, list[int | str | None]] = {"data": [1, "two", None]}

# Class with union types
class Model:
    name: str
    value: int | str | float
    optional: str | None
    items: list[int | str]
    config: dict[str, int | str | None]
    
    def method(self, x: int | str) -> str | None:
        return None

# Real-world patterns
async def api_endpoint(
    id: int | None,
    query: str | None,
    limit: int | None
) -> list[dict[str, int | str | None]] | None:
    return [{"id": 1, "name": "test", "value": None}]
