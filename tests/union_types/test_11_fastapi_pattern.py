# Test 11: FastAPI Pattern
# Category: Real-World Patterns
# Priority: P1 High

# Typical FastAPI endpoint patterns
async def get_user(user_id: int | None) -> dict[str, int | str | None]:
    return {"id": user_id, "name": "Alice", "email": None}

async def get_items(
    skip: int | None,
    limit: int | None,
    q: str | None
) -> list[dict[str, int | str]]:
    return [{"id": 1, "name": "Item 1"}]

async def create_item(
    name: str,
    price: int | float,
    tax: float | None
) -> dict[str, int | str | float | None]:
    return {"name": name, "price": price, "tax": tax}
