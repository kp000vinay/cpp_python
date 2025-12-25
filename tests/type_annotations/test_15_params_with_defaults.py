# Test parameters with annotations (defaults not supported yet)
# Tests: Multiple annotated parameters

def greet(name: str):
    return name

def configure(host: str, port: int, debug: bool):
    pass

def connect(url: str, timeout: int):
    pass
