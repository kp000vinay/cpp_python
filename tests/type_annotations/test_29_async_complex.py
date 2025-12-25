# Test async functions with complex annotations
# Tests: Async functions with various annotation patterns

async def fetch(url: str) -> dict:
    return {}

async def process(data: list[int]) -> list[str]:
    return []

async def transform(items: dict[str]) -> dict[int]:
    return {}

async def get_value() -> int:
    return 42
