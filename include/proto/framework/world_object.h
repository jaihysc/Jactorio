// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
#define JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
#pragma once

#include "proto/framework/framework_base.h"
#include "proto/interface/renderable.h"
#include "proto/interface/serializable.h"

namespace jactorio::data
{
    struct FWorldObjectData : UniqueDataBase, IRenderableData
    {
        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<UniqueDataBase>(this), cereal::base_class<IRenderableData>(this));
        }
    };

    class FWorldObject : public FrameworkBase, public IRenderable, public ISerializable
    {
    public:
        // Number of tiles this entity spans
        PYTHON_PROP_REF_I(uint8_t, tileWidth, 1);
        PYTHON_PROP_REF_I(uint8_t, tileHeight, 1);
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
