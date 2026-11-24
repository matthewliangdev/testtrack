#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

sqlite3* openDatabase(const std::string& filename);

bool executeSQL(sqlite3* db, const std::string& sql);

bool initializeDatabase(sqlite3* db);

void closeDatabase(sqlite3* db);

#endif