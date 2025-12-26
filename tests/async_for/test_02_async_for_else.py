# Test async for loop with else clause
async def process():
    async for x in items:
        print(x)
    else:
        print("done")
