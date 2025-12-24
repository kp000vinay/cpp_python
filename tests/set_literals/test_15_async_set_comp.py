# Test 15: Async set comprehension
async def main():
    x = {i async for i in aiter()}
    y = {item async for item in stream() if item > 0}
