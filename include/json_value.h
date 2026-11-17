#ifndef CURLY_JSON_VALUE_H
#define CURLY_JSON_VALUE_H

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace curly {

// Preserve the original number text without rounding it.
struct JsonNumber {
    std::string text;
};

class JsonValue {
public:
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;

    enum class Type {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    JsonValue() : data_(nullptr) {}
    explicit JsonValue(bool value) : data_(value) {}
    explicit JsonValue(JsonNumber value) : data_(std::move(value)) {}
    explicit JsonValue(std::string value) : data_(std::move(value)) {}

    // Prevent a string literal from being interpreted as a boolean.
    explicit JsonValue(const char* value)
        : data_(checkedString(value)) {}

    explicit JsonValue(Array value) : data_(std::move(value)) {}
    explicit JsonValue(Object value) : data_(std::move(value)) {}

    Type type() const {
        switch (data_.index()) {
            case 0: return Type::Null;
            case 1: return Type::Boolean;
            case 2: return Type::Number;
            case 3: return Type::String;
            case 4: return Type::Array;
            case 5: return Type::Object;
            default:
                throw std::logic_error("JSON value has no active type");
        }
    }

    bool isNull() const {
        return std::holds_alternative<std::nullptr_t>(data_);
    }

    bool asBool() const {
        return get<bool>("a boolean");
    }

    const JsonNumber& asNumber() const {
        return get<JsonNumber>("a number");
    }

    const std::string& asString() const {
        return get<std::string>("a string");
    }

    const Array& asArray() const {
        return get<Array>("an array");
    }

    const Object& asObject() const {
        return get<Object>("an object");
    }

    const JsonValue& at(const std::string& key) const {
        const auto& object = asObject();
        const auto found = object.find(key);

        if (found == object.end()) {
            throw std::out_of_range("Missing JSON key: " + key);
        }

        return found->second;
    }

    const JsonValue& at(std::size_t index) const {
        return asArray().at(index);
    }

private:
    using Storage = std::variant<
        std::nullptr_t,
        bool,
        JsonNumber,
        std::string,
        Array,
        Object
    >;

    Storage data_;

    static std::string checkedString(const char* value) {
        if (value == nullptr) {
            throw std::invalid_argument("Null string pointer");
        }
        return std::string(value);
    }

    template <typename T>
    const T& get(const char* expected) const {
        const T* value = std::get_if<T>(&data_);

        if (value == nullptr) {
            throw std::runtime_error(
                std::string("JSON value is not ") + expected
            );
        }

        return *value;
    }
};

} // namespace curly

#endif
