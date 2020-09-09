// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_CORE_UTILITY_H
#define JACTORIO_CORE_UTILITY_H
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
} // namespace jactorio::core

#endif // JACTORIO_CORE_UTILITY_H
