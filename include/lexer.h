#ifndef JSON_PARSER_LEXER_H
#define JSON_PARSER_LEXER_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

enum class TokenType {
    LeftBrace,
    RightBrace,
    End
};

struct Token {
    TokenType type;
    std::size_t position;
};

class Lexer {
public:
    explicit Lexer(std::string_view input)
        : input_(input) {}

    Token nextToken() {
        skipWhitespace();

        if (position_ == input_.size()) {
            return {TokenType::End, position_};
        }

        const std::size_t start = position_;
        const char character = input_[position_++];

        switch (character) {
            case '{':
                return {TokenType::LeftBrace, start};
            case '}':
                return {TokenType::RightBrace, start};
            default:
                throw std::runtime_error(
                    "Unexpected character at byte " +
                    std::to_string(start)
                );
        }
    }

private:
    std::string_view input_;
    std::size_t position_ = 0;

    void skipWhitespace() {
        while (position_ < input_.size()) {
            const char c = input_[position_];

            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                break;
            }

            ++position_;
        }
    }
};

#endif
