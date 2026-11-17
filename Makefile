CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
HEADERS = $(wildcard include/*.h)

.PHONY: all test test-values example
all: json-parser

json-parser: src/main.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

build:
	mkdir -p build

build/test-parser: tests/test_parser.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

build/test-values: tests/test_json_value.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

build/read-profile: examples/read_profile.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

test-values: build/test-values
	./build/test-values

test: json-parser build/test-parser build/test-values build/read-profile
	./build/test-parser
	./build/test-values
	bash tests/test_cli.sh

example: build/read-profile
	./build/read-profile examples/profile.json
