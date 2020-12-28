// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
#define JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
#pragma once

#include "proto/detail/type.h"
#include "proto/framework/framework_base.h"
#include "proto/interface/renderable.h"
#include "proto/interface/serializable.h"

namespace jactorio::proto
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
        using TileSpanT = uint8_t;

        ///
        /// If true, swaps width and height when orientation is left or right in Getters
        PYTHON_PROP_REF_I(bool, rotateDimensions, true);

        // TODO remove default arg, for compatibility with existing code for now
        J_NODISCARD TileSpanT GetWidth(const Orientation orientation = Orientation::up) const {
            if (rotateDimensions && (orientation == Orientation::left || orientation == Orientation::right))
                return height_;

            return width_;
        }
        FWorldObject* SetWidth(const TileSpanT width) {
            width_ = width;
            return this;
        }

        J_NODISCARD TileSpanT GetHeight(const Orientation orientation = Orientation::up) const {
            if (rotateDimensions && (orientation == Orientation::left || orientation == Orientation::right))
                return width_;

            return height_;
        }
        FWorldObject* SetHeight(const TileSpanT height) {
            height_ = height;
            return this;
        }

    private:
        /// Number of tiles which object occupies
        TileSpanT width_  = 1;
        TileSpanT height_ = 1;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
