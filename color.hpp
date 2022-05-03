/**
 * @file color.hpp
 * @author Jayson Sho Toma
 * @brief calculates the colored string.
 * @version 0.1
 * @date 2022-05-03
 */

#pragma once

#include <string>

 /**
  * @brief Get the string with ANSI-color code enabled.
  *        Available colors are:
  *        - Default
  *        - Black
  *        - Red
  *        - Green
  *        - Yellow
  *        - Magenta
  *        - Cyan
  *        - Light Gray
  *        - Dark Gray
  *        - Light Red
  *        - Light Green
  *        - Light Yellow
  *        - Light Blue
  *        - Light Magenta
  *        - Light Cyan
  *        - White
  *
  * @param text text to be displayed
  * @param color color of the text
  * @return `text` wrapped with ANSI-color code. If `color` does not match with any colors on the list, `text`'s color will fall back to `Default`.
  */
std::string getColoredString(const std::string &text, const std::string &color);
