# Test dictionary literals

# Empty dict
empty_dict = {}

# Simple dict
person = {"name": "Alice", "age": 30}

# Dict with numbers as keys
numbers = {1: "one", 2: "two", 3: "three"}

# Dict with expressions as values
computed = {"sum": x + y, "product": x * y}

# Nested dict
nested = {"outer": {"inner": "value"}}

# Dict with multiple entries
config = {
    "host": "localhost",
    "port": 8080,
    "debug": True
}

# Dict access
name = person["name"]
# Note: Method calls like person.get() are not yet supported
# age = person.get("age", 0)

# Dict in condition
if "name" in person:
    print("Has name")

