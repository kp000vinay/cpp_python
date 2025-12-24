# Test: Async list comprehension with if clause
async def main():
    result = [x async for x in aiter() if x > 0]
