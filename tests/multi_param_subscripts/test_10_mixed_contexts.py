# Test 10: Multi-Param in Mixed Contexts
# Category: Complex Types
# Priority: P1 High
# Expected: PASS after Phase 6 implementation

# Test multi-parameter types in mixed contexts
def apply(func: dict[str, int], x: int) -> int:
    return x

def process(data: tuple[int, str]) -> dict[str, int]:
    return {}
