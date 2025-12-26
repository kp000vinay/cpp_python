# Test async with statement
async def process():
    async with aiofiles.open("file.txt") as f:
        data = await f.read()
