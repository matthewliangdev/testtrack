#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>

sqlite3* openDatabase(const std::string& filename) {
    sqlite3* db = nullptr;

    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to open database: "
                  << sqlite3_errmsg(db) << "\n";

        sqlite3_close(db);
        return nullptr;
    }

    if (!executeSQL(db, "PRAGMA foreign_keys = ON;")) {
        sqlite3_close(db);
        return nullptr;
    }

    return db;
}

bool executeSQL(sqlite3* db, const std::string& sql) {
    char* error = nullptr;

    int result = sqlite3_exec(
        db, sql.c_str(), nullptr, nullptr, &error
    );

    if (result != SQLITE_OK) {
        std::cerr << "SQL error: "
                  << (error ? error : sqlite3_errmsg(db))
                  << "\n";

        sqlite3_free(error);
        return false;
    }

    return true;
}

bool initializeDatabase(sqlite3* db) {
    std::ifstream file("schema.sql");

    if (!file.is_open()) {
        std::cerr << "Could not open schema.sql\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return executeSQL(db, buffer.str());
}

void closeDatabase(sqlite3* db) {
    sqlite3_close(db);
}

static bool insertRun(sqlite3* db,
                      const std::string& source,
                      sqlite3_int64& runID) {
    const char* sql =
        "INSERT INTO test_runs (source) VALUES (?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)
        != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(db) << '\n';
        return false;
    }

    bool success =
        sqlite3_bind_text(stmt, 1, source.c_str(), -1,
                          SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_step(stmt) == SQLITE_DONE;

    if (success)
        runID = sqlite3_last_insert_rowid(db);
    else
        std::cerr << sqlite3_errmsg(db) << '\n';

    sqlite3_finalize(stmt);

    return success;
}

static bool insertResults(
    sqlite3* db,
    sqlite3_int64 runID,
    const std::vector<TestResult>& results) {

    const char* sql =
        "INSERT INTO test_results "
        "(run_id, test_name, status, duration_ms) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)
        != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(db) << '\n';
        return false;
    }

    for (const TestResult& result : results) {
        bool success =
            sqlite3_bind_int64(stmt, 1, runID) == SQLITE_OK &&
            sqlite3_bind_text(stmt, 2, result.test_name.c_str(),
                              -1, SQLITE_TRANSIENT) == SQLITE_OK &&
            sqlite3_bind_text(stmt, 3, result.status.c_str(),
                              -1, SQLITE_TRANSIENT) == SQLITE_OK &&
            sqlite3_bind_double(stmt, 4, result.duration_ms)
                              == SQLITE_OK &&
            sqlite3_step(stmt) == SQLITE_DONE;

        if (!success) {
            std::cerr << sqlite3_errmsg(db) << '\n';
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    return true;
}

bool saveRun(sqlite3* db,
             const std::string& source,
             const std::vector<TestResult>& results) {

    if (results.empty() ||
        !executeSQL(db, "BEGIN TRANSACTION;"))
        return false;

    sqlite3_int64 runID = 0;

    if (!insertRun(db, source, runID) ||
        !insertResults(db, runID, results) ||
        !executeSQL(db, "COMMIT;")) {

        executeSQL(db, "ROLLBACK;");
        return false;
    }

    return true;
}