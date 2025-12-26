# Test yield as expression (for send())
def accumulator():
    total = 0
    while True:
        value = yield total
        total = total + value
