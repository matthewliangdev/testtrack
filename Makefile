CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDLIBS = -lsqlite3

TARGET = testtrack
SRC = src/main.cpp src/database.cpp src/importer.cpp src/reports.cpp
HEADERS = src/database.h src/importer.h src/reports.h

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDLIBS) -o $(TARGET)

build/importer_test: tests/importer_test.cpp src/importer.cpp src/importer.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) tests/importer_test.cpp src/importer.cpp -o $@

test: $(TARGET) build/importer_test
	./build/importer_test
	python3 tests/integration_test.py

clean:
	rm -rf $(TARGET) build

.PHONY: test clean
