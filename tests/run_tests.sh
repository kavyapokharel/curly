#!/bin/bash
set -eu

cd "$(dirname "$0")/.."

clang++ -std=c++17 -Wall -Wextra -Wpedantic -g \
    -Iinclude src/main.cpp -o json-parser

test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT

passed=0
failed=0

check() {
    local expected="$1"
    local label="$2"
    local input="$3"
    local actual

    printf '%s' "$input" > "$test_dir/input.json"

    if ./json-parser "$test_dir/input.json" > "$test_dir/output.txt" 2>&1; then
        actual=0
    else
        actual=$?
    fi

    if [ "$actual" -eq "$expected" ]; then
        printf 'PASS: %s\n' "$label"
        passed=$((passed + 1))
    else
        printf 'FAIL: %s (expected exit %s, got %s)\n' \
            "$label" "$expected" "$actual"
        cat "$test_dir/output.txt"
        failed=$((failed + 1))
    fi
}

check 0 "empty object" '{}'
check 0 "empty array" '[]'
check 0 "string object" '{"name":"Kavya"}'
check 0 "escaped strings" '{"s":"Hello \"Kavya\"\n\\\/\b\f\r\t\u0041"}'
check 0 "booleans and null" '{"a":true,"b":false,"c":null}'
check 0 "number formats" '[0,-0,20,-12,3.14,1e3,1E+3,-2e-4]'
check 0 "nested containers" '{"a":[1,{"b":[true,null,[]]}]}'
check 0 "root string" '"hello"'
check 0 "root number" '42'
check 0 "root true" 'true'
check 0 "root false" 'false'
check 0 "root null" 'null'
check 0 "whitespace" $' \t{\n"x": true\r}\n'

check 1 "empty input" ''
check 1 "whitespace only" $' \n\t'
check 1 "missing colon" '{"a" "b"}'
check 1 "missing comma" '[1 2]'
check 1 "object trailing comma" '{"a":1,}'
check 1 "array trailing comma" '[1,]'
check 1 "missing value" '{"a":}'
check 1 "unquoted key" '{true:false}'
check 1 "extra root value" '{} {}'
check 1 "leading zero" '{"n":01}'
check 1 "missing integer digits" '-'
check 1 "missing fraction digits" '1.'
check 1 "missing exponent digits" '1e+'
check 1 "leading plus sign" '+1'
check 1 "leading decimal point" '.5'
check 1 "number suffix" '12abc'
check 1 "misspelled keyword" '{"a":tru}'
check 1 "keyword suffix" 'truefalse'
check 1 "invalid escape" '{"s":"\q"}'
check 1 "invalid Unicode escape" '{"s":"\u12G4"}'
check 1 "unterminated string" '"hello'
check 1 "unescaped newline" $'{"s":"first\nsecond"}'
check 1 "unfinished array" '[1,2'
check 1 "unfinished object" '{"a":true'
check 1 "mismatched brackets" '{"a":[1,2}'

nested='0'
for ((i = 0; i < 128; i++)); do
    nested="[$nested]"
done

check 0 "128 levels of nesting" "$nested"
check 1 "nesting limit" "[$nested]"

printf '\n%s passed; %s failed\n' "$passed" "$failed"
[ "$failed" -eq 0 ]
