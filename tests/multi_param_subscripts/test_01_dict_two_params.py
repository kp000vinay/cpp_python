# Test 01: dict[K, V] - Basic Two Parameters
# Category: Basic Multi-Parameter Subscripts
# Priority: P0 Critical
# Expected: PASS after Phase 6 implementation

# Test basic dict with two type parameters
data: dict[str, int] = {}
mapping: dict[str, str] = {"key": "value"}
scores: dict[str, float] = {"alice": 95.5}
config: dict[str, bool] = {"debug": True}

# Expected AST for 'data':
# AnnAssign(
#     target=Name('data'),
#     annotation=Subscript(
#         value=Name('dict'),
#         slice=Tuple([Name('str'), Name('int')])
#     ),
#     value=Dict([])
# )
