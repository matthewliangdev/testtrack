#ifndef REPORTS_H
#define REPORTS_H

#include <sqlite3.h>

bool printSummary(sqlite3* db);
bool printFailures(sqlite3* db);
bool printHistory(sqlite3* db);
bool printRegressions(sqlite3* db);

#endif
