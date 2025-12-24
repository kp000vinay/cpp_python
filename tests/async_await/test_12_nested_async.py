# Test: Nested async function definitions
async def outer():
    async def inner():
        return 42
    return await inner()
