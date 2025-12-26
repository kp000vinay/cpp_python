# Test: T-string with complex expressions
# PEP 750: Template String Literals (Python 3.14+)

items = [1, 2, 3]
template = t"Sum: {sum(items)}"
template_len = t"Length: {len(items)}"
template_math = t"Result: {2 + 3 * 4}"
