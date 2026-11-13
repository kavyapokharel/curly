#ifndef JSON_PARSER_LEXER_H
#define JSON_PARSER_LEXER_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

enum class TokenType {
    LeftBrace, RightBrace,
    LeftBracket, RightBracket,
    Colon, Comma,
    String, Number,
    True, False, Null,
    End
};

struct Token {
    TokenType type;
    std::size_t position;
};

class Lexer {
public:
    explicit Lexer(std::string_view input) : input_(input) {}

    Token nextToken() {
        skipWhitespace();

        if (position_ == input_.size()) {
            return {TokenType::End, position_};
        }

        const std::size_t start = position_;
        const char c = input_[position_++];

        switch (c) {
            case '{': return {TokenType::LeftBrace, start};
            case '}': return {TokenType::RightBrace, start};
            case '[': return {TokenType::LeftBracket, start};
            case ']': return {TokenType::RightBracket, start};
            case ':': return {TokenType::Colon, start};
            case ',': return {TokenType::Comma, start};
            case '"': return readString(start);
            case 't': return readLiteral(start, "true", TokenType::True);
            case 'f': return readLiteral(start, "false", TokenType::False);
            case 'n': return readLiteral(start, "null", TokenType::Null);
            default:
                if (c == '-' || isDigit(c)) {
                    position_ = start;
                    return readNumber(start);
                }
                fail("Unexpected character", start);
        }
    }

private:
    std::string_view input_;
    std::size_t position_ = 0;

    [[noreturn]] static void fail(
        const std::string& message, std::size_t position
    ) {
        throw std::runtime_error(
            message + " at byte " + std::to_string(position)
        );
    }

    static bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }

    static bool isWhitespace(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    static bool isHexDigit(char c) {
        return isDigit(c) ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    bool hasDigit() const {
        return position_ < input_.size() && isDigit(input_[position_]);
    }

    void checkValueBoundary() const {
        if (position_ == input_.size()) {
            return;
        }

        const char c = input_[position_];
        if (!isWhitespace(c) && c != ',' && c != '}' && c != ']') {
            fail("Unexpected character after value", position_);
        }
    }

    Token readLiteral(
        std::size_t start, std::string_view word, TokenType type
    ) {
        if (input_.substr(start, word.size()) != word) {
            fail("Expected '" + std::string(word) + "'", start);
        }

        position_ = start + word.size();
        checkValueBoundary();
        return {type, start};
    }

    Token readNumber(std::size_t start) {
        if (input_[position_] == '-') {
            ++position_;
        }

        if (!hasDigit()) {
            fail("Expected a digit", position_);
        }

        if (input_[position_] == '0') {
            ++position_;
            if (hasDigit()) {
                fail("Leading zeros are not allowed", position_);
            }
        } else {
            while (hasDigit()) {
                ++position_;
            }
        }

        if (position_ < input_.size() && input_[position_] == '.') {
            ++position_;

            if (!hasDigit()) {
                fail("Expected a digit after decimal point", position_);
            }

            while (hasDigit()) {
                ++position_;
            }
        }

        if (position_ < input_.size() &&
            (input_[position_] == 'e' || input_[position_] == 'E')) {
            ++position_;

            if (position_ < input_.size() &&
                (input_[position_] == '+' || input_[position_] == '-')) {
                ++position_;
            }

            if (!hasDigit()) {
                fail("Expected an exponent digit", position_);
            }

            while (hasDigit()) {
                ++position_;
            }
        }

        checkValueBoundary();
        return {TokenType::Number, start};
    }

    void readEscape(std::size_t start) {
        if (position_ == input_.size()) {
            fail("Incomplete escape sequence", start);
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
                        fail("Incomplete Unicode escape", start);
                    }
                    if (!isHexDigit(input_[position_])) {
                        fail("Expected a hexadecimal digit", position_);
                    }
                    ++position_;
                }
                return;

            default:
                fail("Invalid escape sequence", start);
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
                fail("Unescaped control character", position_ - 1);
            }
        }

        fail("Unterminated string starting", start);
    }

    void skipWhitespace() {
        while (position_ < input_.size() &&
               isWhitespace(input_[position_])) {
            ++position_;
        }
    }
};

#endif
