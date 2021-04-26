// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_LOCAL_PARSER_H
#define JACTORIO_INCLUDE_DATA_LOCAL_PARSER_H
#pragma once

#include <string>

namespace jactorio::data
{
    class PrototypeManager;

    /// Parses a .cfg file, will throw exceptions on error
    /// \param file_str File contents
    /// \param directory_prefix Added in front when searching for internal names : objectA -> __name__/objectA
    /// \exception ProtoError if parsing failed
    void LocalParse(PrototypeManager& proto, const std::string& file_str, const std::string& directory_prefix);
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_LOCAL_PARSER_H
