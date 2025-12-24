# Test: Basic async dict comprehension
async def main():
    result = {k: v async for k, v in items()}
