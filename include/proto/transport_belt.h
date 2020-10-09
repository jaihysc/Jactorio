// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_TRANSPORT_BELT_H
#define JACTORIO_INCLUDE_PROTO_TRANSPORT_BELT_H
#pragma once

#include "proto/abstract_proto/transport_line.h"

namespace jactorio::proto
{
    class TransportBelt final : public TransportLine
    {
        PROTOTYPE_CATEGORY(transport_belt);
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_TRANSPORT_BELT_H
