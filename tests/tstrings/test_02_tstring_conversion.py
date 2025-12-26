# Test: T-string with conversion specifiers (!s, !r, !a)
# PEP 750: Template String Literals (Python 3.14+)

value = 42
template_s = t"Value: {value!s}"
template_r = t"Value: {value!r}"
template_a = t"Value: {value!a}"
