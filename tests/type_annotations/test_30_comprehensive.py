# Test comprehensive mixed scenarios
# Tests: All annotation features in one file

# Variable annotations
x: int = 5
y: str = "hello"
items: list[int] = [1, 2, 3]
data: dict[str] = {}

# Function with parameters and return
def process(name: str, count: int) -> dict:
    return {}

# Function without return annotation
def helper(value: int):
    return value * 2

# Async function
async def fetch(url: str) -> dict:
    return {}

# Class with methods
class Handler:
    def handle(self, data: dict) -> list:
        return []
    
    def process(self, items: list[int]) -> int:
        return 0
