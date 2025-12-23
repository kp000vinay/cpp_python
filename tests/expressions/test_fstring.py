# Test f-strings (formatted string literals)
# Reference: PEP 498, Python 3.6+

# Basic f-strings
x = f"hello"
y = f"hello {name}"
z = f"{a} + {b} = {a+b}"

# Multiple expressions
result = f"Name: {name}, Age: {age}, Score: {score}"

# Conversion specifiers
obj_str = f"{obj!s}"
obj_repr = f"{obj!r}"
obj_ascii = f"{obj!a}"

# Format specifiers
num_formatted = f"{num:.2f}"
text_formatted = f"{text:>10}"
value_formatted = f"{value:0>5}"

# Combined conversion and format
combined = f"{obj!r:>20}"

# Nested f-strings
nested = f"Outer {f'inner {x}'}"

# Escape sequences
escaped = f"Newline\nTab\tQuote\""

# Double braces (literal braces)
literal_braces = f"{{literal}}"

# Raw f-strings
raw_fstring = rf"path\{name}"

# Empty f-string
empty = f""

# Only expression
only_expr = f"{x}"

# Only literal
only_literal = f"just text"

# Complex format specifiers
complex_format = f"{value:>10.2f}"
complex_format2 = f"{text:<20}"

# F-string with method calls
method_call = f"{obj.method()}"

# F-string with attribute access
attr_access = f"{obj.attr}"

# F-string with indexing
indexing = f"{arr[0]}"

# F-string with function calls
func_call = f"{func(arg1, arg2)}"

# F-string with conditional expressions
conditional = f"{x if condition else y}"

# F-string with comprehensions
comp = f"{[x for x in range(5)]}"

# Triple-quoted f-strings
triple = f"""Multi-line
f-string with {value}"""

# F-string with format spec containing nested f-string
nested_format = f"{value:{width}}"

# F-string with multiple format specifiers
multi_format = f"{a:.2f} and {b:>10}"


