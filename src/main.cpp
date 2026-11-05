#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include "lexer.h"

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

    std::string input{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    try {
    Lexer lexer(input);

    // Our first parser accepts exactly: { } end-of-input.
    auto expect = [&](TokenType expected, const char* description) {
        const Token token = lexer.nextToken();

        if (token.type != expected) {
            throw std::runtime_error(
                std::string("Expected ") + description +
                " at byte " + std::to_string(token.position)
            );
        }
    };

    expect(TokenType::LeftBrace, "'{'");
    expect(TokenType::RightBrace, "'}'");
    expect(TokenType::End, "end of input");

    std::cout << "Valid JSON\n";
} catch (const std::runtime_error& error) {
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
}

    return 0;
}
