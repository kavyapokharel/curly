#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include "parser.h"
int main(int argc, char* argv[]) {
    if (argc != 2) { std::cerr << "Usage: read-profile <profile.json>\n"; return 1; }
    try {
        std::ifstream file(argv[1], std::ios::binary);
        if (!file) throw std::runtime_error("Could not open profile");
        const std::string input{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        if (file.bad()) throw std::runtime_error("Failed to read profile");
        Parser parser(input);
        const auto root = parser.parse();
        std::cout << "Name: " << root.at("name").asString() << '\n';
        std::cout << "Age: " << root.at("age").asNumber().text << '\n';
        std::cout << "First skill: " << root.at("skills").at(std::size_t{0}).asString() << '\n';
        std::cout << "Greeting: " << root.at("greeting").asString() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n'; return 1;
    }
}
