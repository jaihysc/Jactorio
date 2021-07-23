// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <algorithm>

#include "core/utility.h"

using namespace jactorio;

std::string jactorio::StrToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

std::string jactorio::StrToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::string jactorio::StrTrim(std::string str, const std::vector<char>& chars) {
    auto is_trim_char = [&](const char c) { return std::find(chars.begin(), chars.end(), c) != chars.end(); };

    // Trim leading
    {
        std::size_t str_start = 0; // Where actual string begins (non whitespace)

        while (is_trim_char(str[str_start]) && str_start < str.size()) {
            ++str_start;
        }

        str.erase(str.begin(), str.begin() + str_start);
    }

    // Trim trailing
    if (!str.empty()) {
        std::size_t str_end = str.size() - 1; // Actual string ends

        while (is_trim_char(str[str_end]) && str_end > 0) {
            --str_end;
        }

        str.erase(str.begin() + str_end + 1, str.end());
    }

    return str;
}

std::string jactorio::StrTrimWhitespace(const std::string& str) {
    return StrTrim(str, {' ', '\0'});
}

std::string jactorio::StrMatchLen(std::string str, const std::size_t target_len, const char padding) {
    while (str.size() < target_len) {
        str.push_back(padding);
    }
    return str;
}

void jactorio::StrReplaceInPlace(std::string& subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}
