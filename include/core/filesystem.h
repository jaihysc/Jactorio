// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#define JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#pragma once

#include <string>

namespace jactorio
{
    /// Reads file from path as string
    /// \return Empty string if path is invalid
    std::string ReadFile(const std::string& path);
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_FILESYSTEM_H
