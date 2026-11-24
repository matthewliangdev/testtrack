CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDLIBS = -lsqlite3

TARGET = testtrack
SRC = src/main.cpp src/database.cpp

$(TARGET): $(SRC) src/database.h
	$(CXX) $(CXXFLAGS) $(SRC) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean