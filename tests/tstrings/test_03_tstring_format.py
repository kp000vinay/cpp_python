# Test: T-string with format specifier
# PEP 750: Template String Literals (Python 3.14+)

pi = 3.14159
template = t"Pi: {pi:.2f}"
width = 10
template_width = t"Value: {pi:>{width}}"
