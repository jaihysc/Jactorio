// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_FRAMEWORK_ENTITY_H
#define JACTORIO_INCLUDE_PROTO_FRAMEWORK_ENTITY_H
#pragma once

#include "proto/framework/world_object.h"
#include "proto/interface/deferred.h"
#include "proto/interface/update_listener.h"

namespace jactorio::proto
{
    struct FEntityData : FWorldObjectData
    {
        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<FWorldObjectData>(this));
        }
    };

    class FEntity : public FWorldObject, public IDeferred, public IUpdateListener
    {
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_FRAMEWORK_ENTITY_H
