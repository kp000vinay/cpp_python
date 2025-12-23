# Test raise statement
raise ValueError("error")

# Test bare raise (not supported yet - requires exception context)
# raise

raise ValueError("error") from TypeError("cause")
