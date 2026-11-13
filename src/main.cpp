#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "parser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: json-parser <file.json>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: could not open " << argv[1] << '\n';
        return 1;
    }

    try {
        const std::string input{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };

        if (file.bad()) {
            throw std::runtime_error("Failed to read input file");
        }

        Parser parser(input);
        parser.parse();

        std::cout << "Valid JSON\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
