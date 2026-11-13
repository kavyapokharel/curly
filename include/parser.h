#ifndef JSON_PARSER_PARSER_H
#define JSON_PARSER_PARSER_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

#include "lexer.h"

class Parser {
public:
    explicit Parser(std::string_view input)
        : lexer_(input), current_(lexer_.nextToken()) {}

    void parse() {
        parseValue(0);
        expect(TokenType::End, "end of input");
    }

private:
    Lexer lexer_;
    Token current_;
    static constexpr std::size_t maxDepth_ = 128;

    [[noreturn]] void fail(const std::string& message) const {
        throw std::runtime_error(
            message + " at byte " + std::to_string(current_.position)
        );
    }

    void advance() {
        current_ = lexer_.nextToken();
    }

    void expect(TokenType type, const char* description) {
        if (current_.type != type) {
            fail(std::string("Expected ") + description);
        }
        advance();
    }

    void parseValue(std::size_t depth) {
        switch (current_.type) {
            case TokenType::LeftBrace:
                if (depth >= maxDepth_) {
                    fail("Maximum nesting depth exceeded");
                }
                parseObject(depth + 1);
                return;

            case TokenType::LeftBracket:
                if (depth >= maxDepth_) {
                    fail("Maximum nesting depth exceeded");
                }
                parseArray(depth + 1);
                return;

            case TokenType::String:
            case TokenType::Number:
            case TokenType::True:
            case TokenType::False:
            case TokenType::Null:
                advance();
                return;

            default:
                fail("Expected a JSON value");
        }
    }

    void parseObject(std::size_t depth) {
        expect(TokenType::LeftBrace, "'{'");

        if (current_.type == TokenType::RightBrace) {
            advance();
            return;
        }

        while (true) {
            expect(TokenType::String, "a string key");
            expect(TokenType::Colon, "':'");
            parseValue(depth);

            if (current_.type == TokenType::RightBrace) {
                advance();
                return;
            }

            expect(TokenType::Comma, "',' or '}'");
        }
    }

    void parseArray(std::size_t depth) {
        expect(TokenType::LeftBracket, "'['");

        if (current_.type == TokenType::RightBracket) {
            advance();
            return;
        }

        while (true) {
            parseValue(depth);

            if (current_.type == TokenType::RightBracket) {
                advance();
                return;
            }

            expect(TokenType::Comma, "',' or ']'");
        }
    }
};

#endif
