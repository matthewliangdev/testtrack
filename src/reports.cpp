#include "reports.h"
#include <iostream>
#include <iomanip>

// Shared helper for reports that return rows of text/numeric columns.
static bool printRows(sqlite3* db, const char* sql, const char* emptyMessage) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(db) << '\n';
        return false;
    }
    bool hasRows = false;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (!hasRows) {
            for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
                if (i) std::cout << " | ";
                std::cout << sqlite3_column_name(stmt, i);
            }
            std::cout << '\n';
            hasRows = true;
        }
        for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
            if (i) std::cout << " | ";
            const unsigned char* value = sqlite3_column_text(stmt, i);
            std::cout << (value ? reinterpret_cast<const char*>(value) : "NULL");
        }
        std::cout << '\n';
    }
    if (rc != SQLITE_DONE)
        std::cerr << sqlite3_errmsg(db) << '\n';
    else if (!hasRows)
        std::cout << emptyMessage << '\n';
    int finalized = sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && finalized == SQLITE_OK;
}

bool printSummary(sqlite3* db) {
    const char* sql = R"SQL(
        SELECT run_id, COUNT(*),
               SUM(CASE WHEN status = 'PASS' THEN 1 ELSE 0 END),
               SUM(CASE WHEN status = 'FAIL' THEN 1 ELSE 0 END),
               AVG(duration_ms)
        FROM test_results
        WHERE run_id = (SELECT MAX(run_id) FROM test_runs)
        GROUP BY run_id;
    )SQL";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(db) << '\n';
        return false;
    }
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto total = sqlite3_column_int64(stmt, 1);
        auto passed = sqlite3_column_int64(stmt, 2);
        auto failed = sqlite3_column_int64(stmt, 3);
        std::cout << "Run: " << sqlite3_column_int64(stmt, 0) << '\n'
                  << "Total tests: " << total << '\n'
                  << "Passed: " << passed << '\n'
                  << "Failed: " << failed << '\n'
                  << std::fixed << std::setprecision(2)
                  << "Pass rate: " << (100.0 * passed / total) << "%\n"
                  << "Average duration: " << sqlite3_column_double(stmt, 4) << " ms\n";
        rc = sqlite3_step(stmt);
    } else if (rc == SQLITE_DONE) {
        std::cout << "No test results available.\n";
    }
    if (rc != SQLITE_DONE)
        std::cerr << sqlite3_errmsg(db) << '\n';
    int finalized = sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && finalized == SQLITE_OK;
}

bool printFailures(sqlite3* db) {
    return printRows(db, R"SQL(
        SELECT test_name, duration_ms
        FROM test_results
        WHERE run_id = (SELECT MAX(run_id) FROM test_runs)
          AND status = 'FAIL'
        ORDER BY test_name;
    )SQL", "No failures in the latest run (or no runs available).");
}

bool printHistory(sqlite3* db) {
    return printRows(db, R"SQL(
        SELECT r.run_id, r.timestamp, r.source, COUNT(t.result_id) AS total,
               SUM(CASE WHEN t.status = 'PASS' THEN 1 ELSE 0 END) AS passed,
               SUM(CASE WHEN t.status = 'FAIL' THEN 1 ELSE 0 END) AS failed,
               ROUND(AVG(t.duration_ms), 2) AS avg_ms
        FROM test_runs r
        LEFT JOIN test_results t ON r.run_id = t.run_id
        GROUP BY r.run_id
        ORDER BY r.run_id DESC;
    )SQL", "No test runs available.");
}

bool printRegressions(sqlite3* db) {
    // Compare only tests present in both of the two most recent imports.
    return printRows(db, R"SQL(
        SELECT previous.test_name, previous.status AS previous_status,
               current.status AS current_status
        FROM test_results current
        JOIN test_results previous ON current.test_name = previous.test_name
        WHERE current.run_id = (SELECT MAX(run_id) FROM test_runs)
          AND previous.run_id = (
              SELECT run_id FROM test_runs ORDER BY run_id DESC LIMIT 1 OFFSET 1
          )
          AND previous.status = 'PASS' AND current.status = 'FAIL'
        ORDER BY current.test_name;
    )SQL", "No regressions detected (requires two runs with matching tests).");
}
