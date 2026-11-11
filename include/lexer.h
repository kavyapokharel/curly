#ifndef JSON_PARSER_LEXER_H
#define JSON_PARSER_LEXER_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

enum class TokenType {
    LeftBrace,
    RightBrace,
    Colon,
    Comma,
    String,
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
            case ':':
                return {TokenType::Colon, start};
            case ',':
                return {TokenType::Comma, start};
            case '"':
                return readString(start);
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

    static bool isHexDigit(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    void readEscape(std::size_t escapeStart) {
        if (position_ == input_.size()) {
            throw std::runtime_error(
                "Incomplete escape sequence at byte " +
                std::to_string(escapeStart)
            );
        }

        const char escaped = input_[position_++];

        switch (escaped) {
            case '"':
            case '\\':
            case '/':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
                return;

            case 'u':
                for (int i = 0; i < 4; ++i) {
                    if (position_ == input_.size()) {
                        throw std::runtime_error(
                            "Incomplete Unicode escape at byte " +
                            std::to_string(escapeStart)
                        );
                    }

                    if (!isHexDigit(input_[position_])) {
                        throw std::runtime_error(
                            "Expected a hexadecimal digit at byte " +
                            std::to_string(position_)
                        );
                    }

                    ++position_;
                }
                return;

            default:
                throw std::runtime_error(
                    "Invalid escape sequence at byte " +
                    std::to_string(escapeStart)
                );
        }
    }

    Token readString(std::size_t start) {
        while (position_ < input_.size()) {
            const char c = input_[position_++];

            if (c == '"') {
                return {TokenType::String, start};
            }

            if (c == '\\') {
                readEscape(position_ - 1);
                continue;
            }

            if (static_cast<unsigned char>(c) < 0x20) {
                throw std::runtime_error(
                    "Unescaped control character at byte " +
                    std::to_string(position_ - 1)
                );
            }
        }

        throw std::runtime_error(
            "Unterminated string starting at byte " +
            std::to_string(start)
        );
    }

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
