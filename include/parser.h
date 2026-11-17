#ifndef JSON_PARSER_PARSER_H
#define JSON_PARSER_PARSER_H
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include "lexer.h"
#include "json_value.h"
#include "json_string.h"

class Parser {
public:
    // Own input so even Parser(std::string(...)) is safe.
    explicit Parser(std::string_view input)
        : input_(input), lexer_(input_), current_(lexer_.nextToken()) {}
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = delete;
    Parser& operator=(Parser&&) = delete;

    curly::JsonValue parse() {
        auto result = parseValue(0);
        expect(TokenType::End, "end of input");
        return result;
    }
private:
    using Value = curly::JsonValue;
    std::string input_;
    Lexer lexer_;
    Token current_;
    static constexpr std::size_t maxDepth_ = 128;
    [[noreturn]] void fail(const std::string& message) const {
        throw std::runtime_error(message + " at byte " + std::to_string(current_.position));
    }
    void advance() { current_ = lexer_.nextToken(); }
    void expect(TokenType type, const char* description) {
        if (current_.type != type) fail(std::string("Expected ") + description);
        advance();
    }
    std::string takeString() {
        if (current_.type != TokenType::String) fail("Expected a string key");
        auto result = curly::detail::decodeString(
            std::string_view(input_).substr(current_.position, current_.length), current_.position);
        advance();
        return result;
    }
    Value parseValue(std::size_t depth) {
        switch (current_.type) {
            case TokenType::LeftBrace:
                if (depth >= maxDepth_) fail("Maximum nesting depth exceeded");
                return parseObject(depth + 1);
            case TokenType::LeftBracket:
                if (depth >= maxDepth_) fail("Maximum nesting depth exceeded");
                return parseArray(depth + 1);
            case TokenType::String: return Value(takeString());
            case TokenType::Number: {
                curly::JsonNumber number{input_.substr(current_.position, current_.length)};
                advance();
                return Value(std::move(number));
            }
            case TokenType::True: advance(); return Value(true);
            case TokenType::False: advance(); return Value(false);
            case TokenType::Null: advance(); return Value();
            default: fail("Expected a JSON value");
        }
    }
    Value parseObject(std::size_t depth) {
        expect(TokenType::LeftBrace, "'{'");
        Value::Object object;
        if (current_.type == TokenType::RightBrace) { advance(); return Value(std::move(object)); }
        while (true) {
            auto key = takeString();
            expect(TokenType::Colon, "':'");
            auto value = parseValue(depth);
            // Explicit policy: the last occurrence of a decoded key wins.
            object.insert_or_assign(std::move(key), std::move(value));
            if (current_.type == TokenType::RightBrace) { advance(); return Value(std::move(object)); }
            expect(TokenType::Comma, "',' or '}'");
        }
    }
    Value parseArray(std::size_t depth) {
        expect(TokenType::LeftBracket, "'['");
        Value::Array array;
        if (current_.type == TokenType::RightBracket) { advance(); return Value(std::move(array)); }
        while (true) {
            array.push_back(parseValue(depth));
            if (current_.type == TokenType::RightBracket) { advance(); return Value(std::move(array)); }
            expect(TokenType::Comma, "',' or ']'");
        }
    }
};
#endif
