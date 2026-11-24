#include "importer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <exception>

bool importCSV(const std::string& filename,
               std::vector<TestResult>& results) {
    results.clear();
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Could not open " << filename << '\n';
        return false;
    }

    std::string line;
    std::getline(file, line);
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    if (line != "test_name,status,duration_ms") {
        std::cerr << "Invalid CSV header\n";
        return false;
    }

    // Keep the caller's vector empty if any row is invalid.
    std::vector<TestResult> parsed;
    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;

        std::stringstream row(line);
        TestResult result{};
        std::string duration;
        if (std::count(line.begin(), line.end(), ',') != 2 ||
            line.find('"') != std::string::npos ||
            line.find('\0') != std::string::npos ||
            !std::getline(row, result.test_name, ',') ||
            !std::getline(row, result.status, ',') ||
            !std::getline(row, duration) ||
            result.test_name.find_first_not_of(" \t") == std::string::npos ||
            (result.status != "PASS" && result.status != "FAIL")) {
            std::cerr << "Invalid record on line " << lineNumber << '\n';
            return false;
        }
        try {
            std::size_t used = 0;
            result.duration_ms = std::stod(duration, &used);
            if (used != duration.size() || !std::isfinite(result.duration_ms) ||
                result.duration_ms < 0) {
                std::cerr << "Invalid duration on line " << lineNumber << '\n';
                return false;
            }
        }
        catch (const std::exception&) {
            std::cerr << "Invalid duration on line " << lineNumber << '\n';
            return false;
        }
        parsed.push_back(result);
    }
    if (file.bad() || !file.eof() || parsed.empty()) {
        std::cerr << "No valid data or file read error\n";
        return false;
    }
    results.swap(parsed);
    return true;
}
