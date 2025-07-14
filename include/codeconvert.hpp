#pragma once

#include <string>

inline std::string OctalStringToUTF8(const std::string& input) {
    std::string result;
    size_t i = 0;
    while (i < input.length()) {
        if (input[i] == '\\' && i + 1 < input.length()) {
            int val = 0;
            int count = 0;
            ++i;
            while (count < 3 && i < input.length() && input[i] >= '0' && input[i] <= '7') {
                val = val * 8 + (input[i] - '0');
                ++i;
                ++count;
            }
            result.push_back(static_cast<char>(val));
        } else {
            result.push_back(input[i]);
            ++i;
        }
    }
    return result;
}