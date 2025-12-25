# Test 08: Nested in Function Signatures
# Category: Nested Multi-Parameter Subscripts
# Priority: P1 High
# Expected: PASS after Phase 6 implementation

# Test nested multi-parameter types in functions
def process(data: dict[str, list[int]]) -> list[dict[str, int]]:
    return []

def transform(items: list[tuple[int, str]]) -> dict[str, tuple[int, int]]:
    return {}
