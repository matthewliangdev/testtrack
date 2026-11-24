PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS test_runs (
    run_id INTEGER PRIMARY KEY,
    timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    source TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS test_results (
    result_id INTEGER PRIMARY KEY,
    run_id INTEGER NOT NULL,
    test_name TEXT NOT NULL,
    status TEXT NOT NULL CHECK (status IN ('PASS', 'FAIL')),
    duration_ms REAL NOT NULL CHECK (duration_ms >= 0),
    FOREIGN KEY (run_id) REFERENCES test_runs(run_id),
    UNIQUE (run_id, test_name)
);