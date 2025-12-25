# Test 11: FastAPI-Style Annotations
# Category: Real-World Patterns
# Priority: P1 High
# Expected: PASS after Phase 6 implementation

# Test FastAPI-style type annotations
async def get_user(user_id: int) -> dict[str, int]:
    return {"id": user_id}

async def get_users() -> list[dict[str, int]]:
    return []

async def update_user(user_id: int, data: dict[str, str]) -> dict[str, int]:
    return {}
