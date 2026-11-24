#include <iostream>
#include <string>
#include "database.h"

int main(int argc, char* argv[]) {
    if (argc != 2 || std::string(argv[1]) != "init") {
        std::cout << "Usage: ./testtrack init\n";
        return 1;
    }

    sqlite3* db = openDatabase("testtrack.db");

    if (db == nullptr) {
        return 1;
    }

    if (!initializeDatabase(db)) {
        closeDatabase(db);
        return 1;
    }

    std::cout << "Database initialized successfully.\n";

    closeDatabase(db);

    return 0;
}