CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic
CPPFLAGS ?= -Isrc

APP := railway
TEST_APP := tests/test_modules
APP_SOURCES := $(wildcard src/*.cpp)
TEST_SOURCES := tests/test_modules.cpp src/Validation.cpp src/TrainSearch.cpp src/FareCalculator.cpp

.PHONY: all run test clean

all: $(APP)

$(APP): $(APP_SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(APP_SOURCES) -o $@

$(TEST_APP): $(TEST_SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TEST_SOURCES) -o $@

run: $(APP)
	./$(APP)

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	rm -f $(APP) $(APP).exe $(TEST_APP) $(TEST_APP).exe
