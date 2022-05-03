/**
 * @file color.cpp
 * @author Jayson Sho Toma
 * @brief calculates the colored string
 * @version 0.1
 * @date 2022-05-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "color.hpp"

#include <map>

namespace {

std::map<std::string, std::string> colorNameToANSICode = {
    { "Default", "39" },
    { "Black", "30" },
    { "Red", "31" },
    { "Green", "32" },
    { "Yellow", "33" },
    { "Blue", "34" },
    { "Magenta", "35" },
    { "Cyan", "36" },
    { "Light Gray", "37" },
    { "Dark Gray", "90" },
    { "Light Red", "91" },
    { "Light Green", "92" },
    { "Light Yellow", "93" },
    { "Light Blue", "94" },
    { "Light Magenta", "95" },
    { "Light Cyan", "96" },
    { "White", "97" }
};

} // namespace

std::string getColoredString(const std::string &text, const std::string &color)
{
    std::string code = colorNameToANSICode["Default"];
    if (auto it = colorNameToANSICode.find(color); it != std::end(colorNameToANSICode)) {
        code = it->second;
    }
    return
        std::string("\033[0;") +
        code +
        ";" +
        "49m" +
        text +
        "\033[0m";
}
