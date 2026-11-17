# Curly

A hand-written C++17 JSON lexer and recursive-descent parser. Curly reads JSON into an owning value tree and provides checked access to objects, arrays, strings, booleans, null, and exact numeric text. No third-party parsing library is used.

## Build and run

Requires a C++17 compiler, Make, and Bash for CLI tests. On macOS:

```sh
make
./json-parser examples/profile.json
make test
make example
```

On Linux with GCC, use `make CXX=g++ test example`.

The validator prints `Valid JSON` on success, returns 0, and otherwise prints an error to stderr and returns 1. Parser errors report zero-based byte offsets (not Unicode character counts).

## Use from C++

```cpp
#include "parser.h"
#include <iostream>

int main() {
    Parser parser(R"({"name":"Kavya","age":20,"skills":["C++"]})");
    const auto root = parser.parse();
    std::cout << root.at("name").asString() << '\n';
    std::cout << root.at("age").asNumber().text << '\n';
    std::cout << root.at("skills").at(std::size_t{0}).asString() << '\n';
}
```

Compile with `-std=c++17 -Iinclude`. `examples/read_profile.cpp` demonstrates reading a file and accessing its parsed contents.

Keep the owning `JsonValue` alive while using references returned by `at()` and `asString()`/`asArray()`/`asObject()`/`asNumber()`. Wrong-type access raises `std::runtime_error`; missing keys and array indices raise `std::out_of_range`.

## Components

| File | Responsibility |
|---|---|
| `include/lexer.h` | Recognize tokens and validate number/escape syntax; track token spans |
| `include/json_string.h` | Decode escapes, combine UTF-16 surrogate pairs, validate raw UTF-8 |
| `include/parser.h` | Build nested values and enforce document grammar |
| `include/json_value.h` | Own values and provide checked access |
| `src/main.cpp` | File input, validation command, exit status |
| `examples/read_profile.cpp` | Demonstrate actual data retrieval |
| `tests/test_parser.cpp` | Parsing, values, Unicode, malformed input, ownership, nesting |
| `tests/test_json_value.cpp` | Value model and access failures |
| `tests/test_cli.sh` | CLI and file-reading checks |

`Parser` owns a copy of its input. It is noncopyable/nonmovable because its lexer views that owned string. Create a fresh parser per document and call `parse()` once. Returned values own their data independently of the parser.

## Explicit behavior and limits

- Any JSON value may be the root.
- Up to 128 nested objects/arrays are accepted.
- Duplicate object keys use **last value wins**, after escape decoding.
- Object keys are stored in `std::map`, so source key order is not retained.
- Numbers retain exact source text, including exponent spelling and negative zero. They are not automatically converted to floating point; callers must choose and range-check conversions.
- `JsonNumber` is a storage type: constructing one manually does not validate its text. Parsing validates number syntax.
- Strings become UTF-8. Invalid UTF-8, overlong encodings, out-of-range code points, and unpaired surrogate escapes are rejected. This is a deliberate strict Unicode policy.
- A leading UTF-8 BOM is rejected. Comments, trailing commas, NaN, and Infinity are rejected.
- This is an in-memory educational parser, not a streaming parser. File size and total allocation are not capped. It has not undergone a production security audit.
- Serialization, JSONPath queries, arbitrary-precision arithmetic, and automatic numeric conversion are outside this first-release scope.

## Tests

```sh
make test          # 84 parser checks + 16 value checks + 5 CLI checks
make test-values   # only value-model tests
bash tests/run_tests.sh  # compatibility entry point, runs make test
```

The original manually created JSON fixtures can remain in `tests/`; the new suite encodes its cases directly and does not automatically discover every `.json` file.

The supplied GitHub workflow runs the suite on Linux/GCC and macOS/Clang after pushes and pull requests. It uses the official checkout action: https://github.com/actions/checkout. The workflow must be pushed and its first run checked before claiming CI passes.

Local verification for this package used GCC on Linux: all 105 checks passed; the parser suite also passed AddressSanitizer/UndefinedBehaviorSanitizer with leak detection disabled due to the execution environment. Another 200 deterministically generated valid documents were accepted. macOS and hosted CI results are not yet verified.
