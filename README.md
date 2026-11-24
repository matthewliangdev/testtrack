# TestTrack

A command-line software test management system written in C++ using SQLite. TestTrack imports test datasets, stores execution history, generates reports, and automates test collection through Python.

## Features

- CSV parsing and input validation.
- Relational database with prepared statements and transactional imports.
- Test summaries, failure reports, and execution history.
- Regression detection between the two most recent runs.
- Automated test execution and result collection using Python's `unittest`.

## Requirements

C++17, SQLite, Make, and Python 3. Developed for Linux/WSL.

```bash
sudo apt update
sudo apt install build-essential sqlite3 libsqlite3-dev python3
```

## Build and Run

Run commands from the project root:

```bash
make
./testtrack init
./testtrack import data/sample_results.csv
./testtrack summary
./testtrack failures
./testtrack history
```

Import the second sample dataset to demonstrate regression detection:

```bash
./testtrack import data/sample_results_2.csv
./testtrack regressions
```

The two sample CSV files are synthetic datasets. In a fresh database, the first has six passing tests and two failing tests. The second introduces a regression in `division`.

## Commands

| Command | Description |
|---|---|
| `init` | Initialize the database. |
| `import <file.csv>` | Validate and store a dataset as a new run. |
| `summary` | Display counts, pass rate, and average duration for the latest run. |
| `failures` | List failed tests in the latest run. |
| `history` | Show all test runs and their statistics. |
| `regressions` | Find tests that passed in the previous run but failed in the latest run. |

## Automated Testing

Run actual example unit tests, export their measured results to CSV, and import them automatically:

```bash
python3 scripts/run_tests.py
```

Failed tests are recorded as `FAIL` before the script returns a nonzero exit code. The included sample suite tests a small Python example application, not the C++ importer itself.

To test TestTrack's C++ importer and CLI/database integration:

```bash
make test
```

## Project Structure

- `src/` - C++ CLI, database connection, importer, and reporting.
- `schema.sql` - Database tables and constraints.
- `data/` - Sample CSV datasets.
- `scripts/` - Python test automation.
- `tests/` - Example unit tests and TestTrack correctness tests.
- `Makefile` - Build and test commands.

## Data Format and Limitations

CSV files must have the header `test_name,status,duration_ms`. Each row must contain a unique test name within its run, a status of `PASS` or `FAIL`, and a nonnegative finite duration in milliseconds. Quoted fields and embedded commas are not supported.

Each import creates a new run in the local `testtrack.db` database; its timestamp represents the import time (UTC). Reports treat the highest run ID as the latest run. Regression detection compares matching test names in the two latest runs, so those runs should represent the same test suite. Newly added or removed tests are not classified as regressions. Generated databases, executables, and CSV output are excluded from Git.
