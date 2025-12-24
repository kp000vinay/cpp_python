# Test: Basic async set comprehension
async def main():
    result = {x async for x in aiter()}
