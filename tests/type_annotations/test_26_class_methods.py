# Test class method annotations
# Tests: Annotations in class methods

class Calculator:
    def add(self, a: int, b: int) -> int:
        return a + b
    
    def process(self, items: list[int]) -> list[int]:
        return items
    
    def configure(self, name: str) -> None:
        pass

class DataProcessor:
    def transform(self, data: dict) -> dict:
        return data
