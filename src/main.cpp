#include <iostream>
#include <string>
#include "database.h"
#include "importer.h"
#include "reports.h"

static void printUsage() {
    std::cout << "Usage: ./testtrack <command>\n"
              << "  init\n  import <file.csv>\n  summary\n"
              << "  failures\n  history\n  regressions\n";
}

int main(int argc, char* argv[]) {
    std::string command = argc > 1 ? argv[1] : "";
    if (argc == 2 && (command == "--help" || command == "help")) {
        printUsage();
        return 0;
    }
    bool valid = (command == "import" && argc == 3) ||
        (argc == 2 && (command == "init" || command == "summary" ||
         command == "failures" || command == "history" || command == "regressions"));
    if (!valid) {
        printUsage();
        return 1;
    }
    sqlite3* db = openDatabase("testtrack.db");
    if (!db)
        return 1;
    bool success = initializeDatabase(db);
    if (success) {
        if (command == "init") {
            std::cout << "Database initialized successfully.\n";
        } else if (command == "import") {
            std::vector<TestResult> results;
            success = importCSV(argv[2], results) && saveRun(db, argv[2], results);
            if (success)
                std::cout << "Imported " << results.size() << " test results.\n";
        } else if (command == "summary") {
            success = printSummary(db);
        } else if (command == "failures") {
            success = printFailures(db);
        } else if (command == "history") {
            success = printHistory(db);
        } else if (command == "regressions") {
            success = printRegressions(db);
        }
    }
    closeDatabase(db);
    return success ? 0 : 1;
}
