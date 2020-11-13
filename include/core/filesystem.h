// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#define JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#pragma once

#include <string>

namespace jactorio::core
{
    std::string GetExecutingDirectory();

    ///
    /// Replaces ~ at the beginning of the path with the executing directory
    /// \remark If there is no ~ at the beginning of the path, nothing happens
    std::string ResolvePath(const std::string& path);

    ///
    /// \remark Cuts off file names (some/where/executing.exe) becomes (some/where)
    void SetExecutingDirectory(const std::string& directory);

    ///
    /// Reads file from path as string
    /// \return Empty string if path is invalid
    std::string ReadFile(const std::string& path);
} // namespace jactorio::core

#endif // JACTORIO_INCLUDE_CORE_FILESYSTEM_H
