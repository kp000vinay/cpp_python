# Test simple class definition
class MyClass:
    x = 1

# Test class with methods
class Calculator:
    def __init__(self):
        self.value = 0

    def add(self, x):
        self.value = self.value + x
        return self.value

    def get_value(self):
        return self.value

# Test class with inheritance
class Animal:
    def __init__(self, name):
        self.name = name

    def speak(self):
        return "Some sound"

class Dog(Animal):
    def speak(self):
        return "Woof"

# Test class with multiple base classes
class A:
    x = 1

class B:
    y = 2

class C(A, B):
    z = 3

