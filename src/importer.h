#ifndef IMPORTER_H
#define IMPORTER_H

#include <string>
#include <vector>

struct TestResult {
    std::string test_name;
    std::string status;
    double duration_ms;
};

bool importCSV(const std::string& filename,
               std::vector<TestResult>& results);

#endif