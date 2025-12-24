# Test: Async with statement with await in body
async def main():
    async with open_conn() as conn:
        data = await conn.read()
        return data
