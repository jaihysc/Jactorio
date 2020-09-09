// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include "core/utility.h"

using namespace jactorio;

std::string core::StrToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

std::string core::StrToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::string core::StrTrimWhitespace(std::string str) {
    auto is_whitespace_char = [](char c) {
        switch (c) {
        case ' ':
        case '\0':
            return true;

        default:
            return false;
        }
    };


    // Trim leading
    {
        std::size_t str_start = 0; // Where actual string begins (non whitespace)

        while (is_whitespace_char(str[str_start]) && str_start < str.size()) {
            ++str_start;
        }

        str.erase(str.begin(), str.begin() + str_start);
    }

    // Trim trailing
    if (!str.empty()) {
        std::size_t str_end = str.size() - 1; // Actual string ends

        while (is_whitespace_char(str[str_end]) && str_end > 0) {
            --str_end;
        }

        str.erase(str.begin() + str_end + 1, str.end());
    }

    return str;
}
