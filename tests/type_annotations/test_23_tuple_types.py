# Test tuple type annotations
# Tests: Tuple types with subscripts

point: tuple[int] = (0, 0)
triple: tuple[str] = ("a", 1, 1.5)

def get_coords() -> tuple[float]:
    return (0.0, 0.0)

def process(data: tuple[str]):
    pass
