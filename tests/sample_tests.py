import unittest
from demo_app import average, divide, parse_quantity


class SampleTests(unittest.TestCase):
    def test_division(self):
        self.assertEqual(divide(12, 3), 4)

    def test_division_by_zero(self):
        with self.assertRaises(ValueError):
            divide(12, 0)

    def test_quantity(self):
        self.assertEqual(parse_quantity("12"), 12)

    def test_negative_quantity(self):
        with self.assertRaises(ValueError):
            parse_quantity("-1")

    def test_invalid_quantity(self):
        with self.assertRaises(ValueError):
            parse_quantity("abc")

    def test_average(self):
        self.assertEqual(average([2, 4, 6]), 4)

    def test_empty_average(self):
        with self.assertRaises(ValueError):
            average([])


if __name__ == "__main__":
    unittest.main()
