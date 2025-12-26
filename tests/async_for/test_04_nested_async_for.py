# Test nested async for loops
async def process():
    async for outer in outer_iter():
        async for inner in inner_iter():
            print(outer, inner)
