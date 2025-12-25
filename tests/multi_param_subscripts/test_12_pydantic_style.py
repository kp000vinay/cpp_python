# Test 12: Pydantic-Style Model Fields
# Category: Real-World Patterns
# Priority: P1 High
# Expected: PASS after Phase 6 implementation

# Test Pydantic-style model field annotations
class User:
    metadata: dict[str, str]
    scores: dict[str, float]
    tags: list[tuple[str, int]]
    
    def get_data(self) -> dict[str, int]:
        return {}
