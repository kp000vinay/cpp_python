# Test: Async for with await in body
async def process():
    async for item in get_items():
        result = await process_item(item)
        print(result)
