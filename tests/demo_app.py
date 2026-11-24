"""Small example application used to demonstrate real test execution."""


def divide(a, b):
    if b == 0:
        raise ValueError("Cannot divide by zero")
    return a / b


def parse_quantity(value):
    quantity = int(value)
    if quantity < 0:
        raise ValueError("Quantity cannot be negative")
    return quantity


def average(values):
    if not values:
        raise ValueError("Values cannot be empty")
    return sum(values) / len(values)
