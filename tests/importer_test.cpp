#include "../src/importer.h"
#include <iostream>
#include <cassert>

int main() {
    std::vector<TestResult> results;
    assert(importCSV("data/sample_results.csv", results));
    assert(results.size() == 8);
    assert(results[0].test_name == "addition");
    assert(results[0].status == "PASS");
    assert(results[0].duration_ms == 2.4);
    std::cout << "Importer tests passed.\n";
    return 0;
}
