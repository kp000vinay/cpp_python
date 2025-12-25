# Test 15: Mixed Single and Multi-Parameter
# Category: Edge Cases
# Priority: P2 Medium
# Expected: PASS after Phase 6 implementation

# Test mixing single and multi-parameter subscripts
# Single parameter (existing functionality)
items: list[int] = []

# Multi-parameter (new functionality)
mapping: dict[str, int] = {}
point: tuple[int, int] = (0, 0)

# Nested mix
data: dict[str, list[int]] = {}
nested: list[dict[str, tuple[int, int]]] = []
