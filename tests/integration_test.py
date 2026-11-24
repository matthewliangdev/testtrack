"""Exercise the CLI with a temporary database; preserve the user's history."""
from pathlib import Path
import shutil
import sqlite3
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class IntegrationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        shutil.copy(ROOT / "schema.sql", self.directory)

    def cli(self, *args, success=True):
        result = subprocess.run([str(ROOT / "testtrack"), *map(str, args)],
                                cwd=self.directory, capture_output=True, text=True)
        if success:
            self.assertEqual(result.returncode, 0, result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0)
        return result.stdout + result.stderr

    def counts(self):
        with sqlite3.connect(self.directory / "testtrack.db") as db:
            return tuple(db.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]
                         for table in ("test_runs", "test_results"))

    def write_csv(self, name, rows):
        path = self.directory / name
        path.write_text("test_name,status,duration_ms\n" + rows, encoding="utf-8")
        return path

    def test_init_and_empty_reports(self):
        self.cli("init")
        self.cli("init")
        self.assertEqual(self.counts(), (0, 0))
        for command in ("summary", "failures", "history", "regressions"):
            self.assertIn("No ", self.cli(command))

    def test_import_and_reports(self):
        self.cli("import", ROOT / "data/sample_results.csv")
        self.assertEqual(self.counts(), (1, 8))
        self.assertIn("75.00%", self.cli("summary"))
        self.assertIn("input_validation", self.cli("failures"))
        self.assertIn("sample_results.csv", self.cli("history"))
        self.assertIn("No regressions", self.cli("regressions"))
        self.cli("import", ROOT / "data/sample_results_2.csv")
        self.assertEqual(self.counts(), (2, 16))
        self.assertIn("division | PASS | FAIL", self.cli("regressions"))
        self.assertNotIn("empty_input", self.cli("regressions"))
        self.assertNotIn("input_validation", self.cli("regressions"))
        self.assertIn("sample_results_2.csv", self.cli("history"))

    def test_invalid_inputs(self):
        self.cli("init")
        for row in ("a,PASS,2abc\n", "a,UNKNOWN,2\n", "a,PASS,-2\n",
                    "a,PASS,nan\n", "a,PASS,inf\n", ",PASS,2\n",
                    "a,PASS,2,extra\n", '"a",PASS,2\n', ""):
            with self.subTest(row=row):
                self.cli("import", self.write_csv("bad.csv", row), success=False)
                self.assertEqual(self.counts(), (0, 0))
        self.cli("import", self.directory / "missing.csv", success=False)
        self.assertEqual(self.counts(), (0, 0))

    def test_transaction_rollback(self):
        self.cli("import", ROOT / "data/sample_results.csv")
        duplicate = self.write_csv("duplicate.csv", "a,PASS,2\na,FAIL,3\n")
        self.assertIn("UNIQUE constraint failed", self.cli("import", duplicate, success=False))
        self.assertEqual(self.counts(), (1, 8))

    def test_sql_binding(self):
        name = "x'); DROP TABLE test_runs; --"
        path = self.write_csv("literal.csv", name + ",PASS,1\n")
        self.cli("import", path)
        with sqlite3.connect(self.directory / "testtrack.db") as db:
            self.assertEqual(db.execute("SELECT test_name FROM test_results").fetchone()[0], name)
        self.assertEqual(self.counts(), (1, 1))

    def test_regression_ignores_missing_tests(self):
        self.cli("import", self.write_csv("first.csv", "a,PASS,1\nb,PASS,1\n"))
        self.cli("import", self.write_csv("second.csv", "b,FAIL,1\nc,FAIL,1\n"))
        self.assertIn("b | PASS | FAIL", self.cli("regressions"))
        self.assertNotIn("c |", self.cli("regressions"))
        self.assertNotIn("a |", self.cli("regressions"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
