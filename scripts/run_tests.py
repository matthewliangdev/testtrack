"""Run the example test suite, export its results, and import into TestTrack."""
import csv
from pathlib import Path
import subprocess
import time
import unittest

ROOT = Path(__file__).resolve().parents[1]


class RecordingResult(unittest.TextTestResult):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.rows = []

    def startTest(self, test):
        self.started = time.perf_counter()
        super().startTest(test)

    def stopTest(self, test):
        super().stopTest(test)

    def record(self, test, status):
        duration = (time.perf_counter() - self.started) * 1000
        self.rows.append((test.id(), status, f"{duration:.6f}"))

    def addSuccess(self, test):
        self.record(test, "PASS")
        super().addSuccess(test)

    def addFailure(self, test, err):
        self.record(test, "FAIL")
        super().addFailure(test, err)

    def addError(self, test, err):
        # Discovery errors may occur outside a normal test lifecycle.
        if hasattr(self, "started"):
            self.record(test, "FAIL")
        super().addError(test, err)


def main():
    executable = ROOT / "testtrack"
    if not executable.is_file():
        print("Build TestTrack with make first.")
        return 1

    suite = unittest.defaultTestLoader.discover(
        str(ROOT / "tests"), pattern="sample_tests.py"
    )
    result = unittest.TextTestRunner(
        verbosity=2, resultclass=RecordingResult
    ).run(suite)

    # The current database supports only PASS and FAIL. Do not silently
    # import incomplete suites containing skips or other unsupported outcomes.
    if (not result.rows or len(result.rows) != result.testsRun or
            result.skipped or result.expectedFailures or result.unexpectedSuccesses or
            len({row[0] for row in result.rows}) != len(result.rows)):
        print("No complete PASS/FAIL dataset was produced.")
        return 1

    csv_path = ROOT / "data" / "results.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.writer(file)
        writer.writerow(["test_name", "status", "duration_ms"])
        writer.writerows(result.rows)

    try:
        subprocess.run([str(executable), "import", "data/results.csv"],
                       cwd=ROOT, check=True)
        subprocess.run([str(executable), "summary"], cwd=ROOT, check=True)
    except subprocess.CalledProcessError:
        return 1

    # Failed tests are recorded in the database, but the script still signals
    # failure to callers such as CI pipelines.
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
