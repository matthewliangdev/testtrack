#include <iostream>
#include <string>
#include "database.h"
#include "importer.h"

int main(int argc, char* argv[]) {
    std::string command = argc > 1 ? argv[1] : "";

    if (!((command == "init" && argc == 2) ||
          (command == "import" && argc == 3))) {
        std::cerr << "Usage: ./testtrack init\n"
                  << "       ./testtrack import <file.csv>\n";
        return 1;
    }

    sqlite3* db = openDatabase("testtrack.db");

    if (!db)
        return 1;

    bool success = initializeDatabase(db);

    if (success && command == "init") {
        std::cout << "Database initialized successfully.\n";
    }
    else if (success && command == "import") {
        std::vector<TestResult> results;

        success = importCSV(argv[2], results) &&
                  saveRun(db, argv[2], results);

        if (success) {
            std::cout << "Imported " << results.size()
                      << " test results.\n";
        }
    }

    closeDatabase(db);

    return success ? 0 : 1;
}