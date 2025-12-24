# Test 11: Walrus in elif statement
if (x := get_x()) > 10:
    print("big")
elif (y := get_y()) > 5:
    print("medium")
