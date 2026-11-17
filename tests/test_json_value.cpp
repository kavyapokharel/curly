#include <iostream>
#include <stdexcept>
#include "json_value.h"
using curly::JsonValue;
using curly::JsonNumber;
int count = 0;
void check(bool ok) { if (!ok) throw std::runtime_error("Value test failed"); ++count; }
template<class E, class F> void throws(F f) {
    try { f(); } catch (const E&) { ++count; return; }
    throw std::runtime_error("Expected exception was not thrown");
}
int main() {
    try {
        JsonValue root(JsonValue::Object{{"name",JsonValue("Kavya")},{"age",JsonValue(JsonNumber{"20"})},
            {"skills",JsonValue(JsonValue::Array{JsonValue("C++")})}});
        check(root.type() == JsonValue::Type::Object);
        check(root.at("name").asString() == "Kavya");
        check(root.at("age").asNumber().text == "20");
        check(root.at("skills").at(std::size_t{0}).asString() == "C++");
        check(JsonValue().isNull()); check(JsonValue(true).asBool()); check(!JsonValue(false).asBool());
        check(JsonValue("hello").type() == JsonValue::Type::String);
        check(JsonValue(JsonNumber{"9007199254740993"}).asNumber().text == "9007199254740993");
        check(JsonValue(JsonValue::Array{}).asArray().empty());
        check(JsonValue(JsonValue::Object{}).asObject().empty());
        throws<std::runtime_error>([&]{root.at("name").asBool();});
        throws<std::out_of_range>([&]{root.at("missing");});
        throws<std::out_of_range>([&]{root.at("skills").at(std::size_t{9});});
        throws<std::invalid_argument>([]{JsonValue v(static_cast<const char*>(nullptr));});
        auto copy = root; check(copy.at("name").asString() == "Kavya");
        std::cout << count << " value checks passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
