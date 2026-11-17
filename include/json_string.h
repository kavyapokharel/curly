#ifndef CURLY_JSON_STRING_H
#define CURLY_JSON_STRING_H
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
namespace curly {
namespace detail {
[[noreturn]] inline void stringError(const char* message, std::size_t offset) {
    throw std::runtime_error(std::string(message) + " at byte " + std::to_string(offset));
}
inline void appendUtf8(std::string& out, std::uint32_t cp) {
    if (cp <= 0x7f) out.push_back(static_cast<char>(cp));
    else if (cp <= 0x7ff) {
        out.push_back(static_cast<char>(0xc0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp <= 0xffff) {
        out.push_back(static_cast<char>(0xe0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else {
        out.push_back(static_cast<char>(0xf0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
}
// Input includes both quotes; base is its byte offset in the document.
inline std::string decodeString(std::string_view raw, std::size_t base) {
    if (raw.size() < 2 || raw.front() != '"' || raw.back() != '"')
        stringError("Invalid quoted string", base);
    std::string out;
    const std::size_t end = raw.size() - 1;
    std::size_t i = 1;
    auto hex4 = [&]() {
        std::uint32_t cp = 0;
        for (int n = 0; n < 4; ++n) {
            if (i >= end) stringError("Incomplete Unicode escape", base + i);
            const char h = raw[i++];
            unsigned digit;
            if (h >= '0' && h <= '9') digit = h - '0';
            else if (h >= 'a' && h <= 'f') digit = h - 'a' + 10;
            else if (h >= 'A' && h <= 'F') digit = h - 'A' + 10;
            else stringError("Invalid Unicode hex digit", base + i - 1);
            cp = (cp << 4) | digit;
        }
        return cp;
    };
    while (i < end) {
        const std::size_t start = i;
        const auto c = static_cast<unsigned char>(raw[i++]);
        if (c == '\\') {
            if (i >= end) stringError("Incomplete escape", base + start);
            switch (raw[i++]) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    auto cp = hex4();
                    if (cp >= 0xd800 && cp <= 0xdbff) {
                        if (i + 2 > end || raw[i] != '\\' || raw[i + 1] != 'u')
                            stringError("Expected low-surrogate escape", base + i);
                        i += 2;
                        const auto low = hex4();
                        if (low < 0xdc00 || low > 0xdfff)
                            stringError("Invalid low surrogate", base + i - 4);
                        cp = 0x10000 + ((cp - 0xd800) << 10) + low - 0xdc00;
                    } else if (cp >= 0xdc00 && cp <= 0xdfff) {
                        stringError("Unpaired low surrogate", base + start);
                    }
                    appendUtf8(out, cp);
                    break;
                }
                default: stringError("Invalid escape", base + start);
            }
        } else if (c < 0x20 || c == '"') {
            stringError("Invalid unescaped string character", base + start);
        } else if (c < 0x80) {
            out.push_back(static_cast<char>(c));
        } else {
            unsigned count;
            std::uint32_t cp, minimum;
            if (c >= 0xc2 && c <= 0xdf) { count = 1; cp = c & 0x1f; minimum = 0x80; }
            else if (c >= 0xe0 && c <= 0xef) { count = 2; cp = c & 0x0f; minimum = 0x800; }
            else if (c >= 0xf0 && c <= 0xf4) { count = 3; cp = c & 7; minimum = 0x10000; }
            else stringError("Invalid UTF-8 leading byte", base + start);
            for (unsigned n = 0; n < count; ++n) {
                if (i >= end) stringError("Truncated UTF-8 sequence", base + start);
                const auto next = static_cast<unsigned char>(raw[i++]);
                if ((next & 0xc0) != 0x80) stringError("Invalid UTF-8 continuation", base + i - 1);
                cp = (cp << 6) | (next & 0x3f);
            }
            if (cp < minimum || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
                stringError("Invalid UTF-8 code point", base + start);
            out.append(raw.substr(start, i - start));
        }
    }
    return out;
}
} // namespace detail
} // namespace curly
#endif
