# Test: Mixed sync and async for clauses
async def main():
    result = [x async for x in aiter() for y in range(10)]
