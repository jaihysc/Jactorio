// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_EXCEPTION_H
#define JACTORIO_INCLUDE_PROTO_EXCEPTION_H
#pragma once

#include <stdexcept>

namespace jactorio::proto
{
    // These are raised by prototype classes if an error occurred
    class ProtoError final : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_EXCEPTION_H
