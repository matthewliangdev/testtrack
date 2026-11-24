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