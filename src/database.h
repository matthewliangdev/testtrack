#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include "importer.h"

sqlite3* openDatabase(const std::string& filename);

bool executeSQL(sqlite3* db, const std::string& sql);

bool initializeDatabase(sqlite3* db);

void closeDatabase(sqlite3* db);

bool saveRun(sqlite3* db,
             const std::string& source,
             const std::vector<TestResult>& results);
#endif