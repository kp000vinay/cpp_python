# Test: Async comprehension with await in expression
async def main():
    result = [await process(x) async for x in aiter()]
