# Test: Nested async comprehension
async def main():
    result = [x async for x in aiter() async for y in biter()]
