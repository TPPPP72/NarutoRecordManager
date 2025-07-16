#pragma once

#include <string>
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

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

inline std::string utf8_to_gbk(const std::string& utf8_str) {
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
    if (wide_len <= 0) return "";

    std::vector<wchar_t> wide_str(wide_len);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, wide_str.data(), wide_len);

    int gbk_len = WideCharToMultiByte(936, 0, wide_str.data(), -1, nullptr, 0, nullptr, nullptr);
    if (gbk_len <= 0) return "";

    std::vector<char> gbk_str(gbk_len);
    WideCharToMultiByte(936, 0, wide_str.data(), -1, gbk_str.data(), gbk_len, nullptr, nullptr);

    return std::string(gbk_str.data());
}

inline long long to_timestamp(const std::string& datetime_str) {
    std::tm tm{};
    std::istringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y_%m_%dT%H_%M_%S");
    if (ss.fail()) {
        return 0;
    }
    time_t timestamp = std::mktime(&tm);
    if (timestamp == -1) {
        return 0;
    }
    return static_cast<long long>(timestamp);
}