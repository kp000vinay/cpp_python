# Test simple async for loop
async def process():
    async for item in async_iter():
        print(item)
