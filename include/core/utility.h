// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_UTILITY_H
#define JACTORIO_INCLUDE_CORE_UTILITY_H
#pragma once

#include <algorithm>
#include <array>
#include <string>

#include "jactorio.h"

namespace jactorio::core
{
    ///
    /// Converts provided string to lower case
    J_NODISCARD std::string StrToLower(std::string str);

    ///
    /// Converts provided string to lower case
    J_NODISCARD std::string StrToUpper(std::string str);

    ///
    /// Remove leading and trailing whitespace ' ' and '\0'
    J_NODISCARD std::string StrTrimWhitespace(std::string str);

    ///
    /// If str is less than target_len, it is padded to target_len with char padding
    J_NODISCARD std::string StrMatchLen(std::string str, std::size_t target_len, char padding = ' ');
} // namespace jactorio::core

#endif // JACTORIO_INCLUDE_CORE_UTILITY_H
