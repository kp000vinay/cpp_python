# Test 12: Pydantic Pattern
# Category: Real-World Patterns
# Priority: P1 High

# Typical Pydantic model field patterns
class User:
    name: str
    age: int | None
    email: str | None
    active: bool | None
    
class Item:
    title: str
    price: int | float
    tax: float | None
    tags: list[str | int]
    
class Response:
    status: int | str
    data: dict[str, int | str | None] | None
    errors: list[str] | None
