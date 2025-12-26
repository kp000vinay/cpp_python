# Test async for loop with tuple unpacking
async def process():
    async for key, value in async_items():
        print(key, value)
