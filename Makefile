CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude

.PHONY: all test

all: json-parser

json-parser: src/main.cpp include/lexer.h include/parser.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp -o json-parser

test:
	bash tests/run_tests.sh

.PHONY: test-values

test-values:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/test_json_value.cpp -o /tmp/curly-value-tests
	/tmp/curly-value-tests
