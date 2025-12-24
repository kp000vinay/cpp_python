# Test: Basic async with statement
async def main():
    async with open_conn() as conn:
        pass
