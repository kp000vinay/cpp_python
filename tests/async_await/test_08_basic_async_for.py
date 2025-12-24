# Test: Basic async for loop
async def process():
    async for item in items():
        print(item)
