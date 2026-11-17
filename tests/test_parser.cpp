#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "parser.h"
int passed = 0;
void check(bool condition, const char* label) {
    if (!condition) throw std::runtime_error(std::string("FAIL: ") + label);
    ++passed;
}
curly::JsonValue parse(std::string_view text) { return Parser(text).parse(); }
void reject(const std::string& text) {
    try { (void)parse(text); }
    catch (const std::runtime_error& e) {
        check(std::string(e.what()).find(" at byte ") != std::string::npos, "position in parse error");
        return;
    }
    throw std::runtime_error("FAIL: accepted invalid input: " + text);
}
int main() {
    try {
        for (const auto& text : std::vector<std::string>{
            "{}", "[]", "null", "true", "false", "42", "-0", "3.14", "1E+3", "-2e-4",
            "\"hello\"", " \t{\n\"x\": true\r}\n",
            R"({"a":[1,{"b":[true,null,[]]}]})"
        }) { (void)parse(text); ++passed; }
        const auto root = parse(R"({"name":"Kavya","age":20,"active":true,"no":false,"mentor":null,"skills":["C++","Python"],"research":{"samples":102}})");
        check(root.at("name").asString() == "Kavya", "string value");
        check(root.at("age").asNumber().text == "20", "number value");
        check(root.at("active").asBool(), "true value");
        check(!root.at("no").asBool(), "false value");
        check(root.at("mentor").isNull(), "null value");
        check(root.at("skills").at(std::size_t{1}).asString() == "Python", "array value");
        check(root.at("research").at("samples").asNumber().text == "102", "nested value");
        check(parse("9007199254740993").asNumber().text == "9007199254740993", "large integer");
        check(parse("1.25e+3000").asNumber().text == "1.25e+3000", "large exponent preserved");
        check(parse("-0.00E-02").asNumber().text == "-0.00E-02", "number spelling preserved");
        check(parse(R"("\"\\\/\b\f\n\r\t")").asString() == std::string("\"\\/\b\f\n\r\t"), "all short escapes");
        check(parse(R"("\u0041\u00e9\u20ac")").asString() == u8"Aé€", "BMP Unicode");
        check(parse(R"("\uD83D\uDE00")").asString() == u8"😀", "surrogate pair");
        check(parse(u8"\"नमस्ते 😀\"").asString() == u8"नमस्ते 😀", "raw UTF-8");
        check(parse(R"("a\u0000b")").asString() == std::string("a\0b", 3), "embedded null");
        check(parse(R"({"\u0061":1,"a":2})").at("a").asNumber().text == "2", "decoded duplicate keys last wins");
        check(parse("[]").asArray().empty(), "empty array");
        check(parse("{}").asObject().empty(), "empty object");
        auto owned = [] { std::string s = R"({"s":"kept"})"; return Parser(s).parse(); }();
        check(owned.at("s").asString() == "kept", "result owns strings");
        Parser fromTemporary(std::string(R"({"a":1})"));
        check(fromTemporary.parse().at("a").asNumber().text == "1", "parser owns input");
        for (const auto& text : std::vector<std::string>{
            "", " \n\t", R"({"a" "b"})", "[1 2]", R"({"a":1,})", "[1,]",
            R"({"a":})", "{true:false}", "{} {}", "01", "-01", "-", "1.", "1e+",
            "+1", ".5", "12abc", "tru", "truefalse", "False", "NaN", "Infinity",
            R"("\q")", R"("\u12G4")", "\"hello", "\"a\nb\"", "[1,2", R"({"a":true)",
            R"({"a":[1,2})", R"("\uD800")", R"("\uDC00")", R"("\uD800\u0041")",
            R"("\uD800\uD800")", R"("\u12")", "\"\\", "/*x*/null", "[,,]", "[1:2]"
        }) reject(text);
        for (const auto& bytes : std::vector<std::string>{
            std::string("\x80",1), std::string("\xc0\xaf",2), std::string("\xe0\x80\xaf",3),
            std::string("\xed\xa0\x80",3), std::string("\xf4\x90\x80\x80",4),
            std::string("\xf5\x80\x80\x80",4), std::string("\xe2\x82",2),
            std::string("\xc2" "A",2), std::string("\xff",1), std::string("\0",1)
        }) reject("\"" + bytes + "\"");
        reject(std::string("\xef\xbb\xbf") + "{}");
        std::string deep = "0";
        for (int i = 0; i < 128; ++i) deep = "[" + deep + "]";
        (void)parse(deep); ++passed;
        reject("[" + deep + "]");
        std::cout << passed << " parser checks passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
