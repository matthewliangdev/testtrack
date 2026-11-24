CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDLIBS = -lsqlite3

TARGET = testtrack
SRC = src/main.cpp src/database.cpp src/importer.cpp
HEADERS = src/database.h src/importer.h

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean