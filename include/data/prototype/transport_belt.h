// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_BELT_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_BELT_H
#pragma once

#include "data/prototype/abstract_proto/transport_line.h"

namespace jactorio::data
{
    class TransportBelt final : public TransportLine
    {
        PROTOTYPE_CATEGORY(transport_belt);
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_BELT_H
