#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
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
        Token current = lexer.nextToken();

        auto expect = [&](TokenType expected, const char* description) {
            if (current.type != expected) {
                throw std::runtime_error(
                    std::string("Expected ") + description +
                    " at byte " + std::to_string(current.position)
                );
            }

            current = lexer.nextToken();
        };

        expect(TokenType::LeftBrace, "'{'");

        if (current.type != TokenType::RightBrace) {
            while (true) {
                expect(TokenType::String, "a string key");
                expect(TokenType::Colon, "':'");
                expect(TokenType::String, "a string value");

                if (current.type != TokenType::Comma) {
                    break;
                }

                expect(TokenType::Comma, "','");
            }
        }

        expect(TokenType::RightBrace, "'}'");
        expect(TokenType::End, "end of input");

        std::cout << "Valid JSON\n";
    } catch (const std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
