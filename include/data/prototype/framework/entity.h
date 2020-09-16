// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
#pragma once

#include "data/prototype/framework/world_object.h"
#include "data/prototype/interface/deferred.h"
#include "data/prototype/interface/rotatable.h"
#include "data/prototype/interface/update_listener.h"

namespace jactorio::data
{
    struct FEntityData : FWorldObjectData
    {
        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<FWorldObjectData>(this));
        }
    };

    class FEntity : public FWorldObject, public IRotatable, public IDeferred, public IUpdateListener
    {
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
