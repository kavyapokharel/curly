#include <iostream>
#include <stdexcept>
#include <string>

#include "json_value.h"

using curly::JsonNumber;
using curly::JsonValue;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("FAIL: " + message);
    }
}

template <typename Exception, typename Function>
void expectThrows(Function action, const std::string& message) {
    try {
        action();
    } catch (const Exception&) {
        return;
    }

    throw std::runtime_error("FAIL: " + message);
}

int main() {
    try {
        JsonValue root(JsonValue::Object{
            {"name", JsonValue("Kavya")},
            {"age", JsonValue(JsonNumber{"20"})},
            {"active", JsonValue(true)},
            {"mentor", JsonValue()},
            {"skills", JsonValue(JsonValue::Array{
                JsonValue("C++"),
                JsonValue("Python")
            })},
            {"research", JsonValue(JsonValue::Object{
                {"samples", JsonValue(JsonNumber{"102"})}
            })}
        });

        require(root.type() == JsonValue::Type::Object, "object type");
        require(root.at("name").asString() == "Kavya", "read string");
        require(root.at("age").asNumber().text == "20", "read number");
        require(root.at("active").asBool(), "read boolean");
        require(root.at("mentor").isNull(), "read null");

        require(
            root.at("skills").at(std::size_t{0}).asString() == "C++",
            "read array element"
        );

        require(
            root.at("research").at("samples").asNumber().text == "102",
            "read nested object"
        );

        JsonValue large(JsonNumber{"9007199254740993"});
        require(
            large.asNumber().text == "9007199254740993",
            "preserve large integer exactly"
        );

        JsonValue exponent(JsonNumber{"1.25e+30"});
        require(
            exponent.asNumber().text == "1.25e+30",
            "preserve exponent notation"
        );

        JsonValue text("hello");
        require(text.type() == JsonValue::Type::String, "literal is a string");

        JsonValue disabled(false);
        require(!disabled.asBool(), "preserve false");

        JsonValue emptyArray(JsonValue::Array{});
        JsonValue emptyObject(JsonValue::Object{});
        require(emptyArray.asArray().empty(), "empty array");
        require(emptyObject.asObject().empty(), "empty object");

        expectThrows<std::runtime_error>(
            [&] { root.at("name").asBool(); },
            "reject wrong type access"
        );

        expectThrows<std::out_of_range>(
            [&] { root.at("missing"); },
            "reject missing key"
        );

        expectThrows<std::out_of_range>(
            [&] { root.at("skills").at(std::size_t{99}); },
            "reject out-of-range index"
        );

        expectThrows<std::invalid_argument>(
            [] { JsonValue value(static_cast<const char*>(nullptr)); },
            "reject null string pointer"
        );

        JsonValue copied = root;
        require(
            copied.at("name").asString() == "Kavya",
            "copy nested document"
        );

        std::cout << "Name: " << root.at("name").asString() << '\n';
        std::cout << "Age: " << root.at("age").asNumber().text << '\n';
        std::cout << "First skill: "
                  << root.at("skills").at(std::size_t{0}).asString()
                  << '\n';
        std::cout << "All JSON value tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
