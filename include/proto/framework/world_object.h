// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
#define JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
#pragma once

#include "core/orientation.h"
#include "proto/framework/framework_base.h"
#include "proto/interface/renderable.h"
#include "proto/interface/serializable.h"

namespace jactorio::proto
{
    struct FWorldObjectData : UniqueDataBase
    {
        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<UniqueDataBase>(this));
        }
    };

    class FWorldObject : public FrameworkBase, public IRenderable, public ISerializable
    {
    public:
        /// If true, swaps width and height when orientation is left or right in Getters
        PYTHON_PROP_REF_I(bool, rotateDimensions, true);

        J_NODISCARD DimensionAxis GetWidth(const Orientation orientation) const {
            if (rotateDimensions && (orientation == Orientation::left || orientation == Orientation::right))
                return dimension_.y;

            return dimension_.x;
        }
        FWorldObject* SetWidth(const DimensionAxis width) {
            dimension_.x = width;
            return this;
        }

        J_NODISCARD DimensionAxis GetHeight(const Orientation orientation) const {
            if (rotateDimensions && (orientation == Orientation::left || orientation == Orientation::right))
                return dimension_.x;

            return dimension_.y;
        }
        FWorldObject* SetHeight(const DimensionAxis height) {
            dimension_.y = height;
            return this;
        }

        J_NODISCARD Dimension GetDimension(const Orientation orientation) const noexcept {
            return {GetWidth(orientation), GetHeight(orientation)};
        }
        /// Calls SetWidth and SetHeight
        FWorldObject* SetDimension(const Dimension& dimension) {
            SetWidth(dimension.x);
            SetHeight(dimension.y);
            return this;
        }

        J_NODISCARD SpriteTexCoordIndexT OnGetTexCoordId(const game::World& world,
                                                         const WorldCoord& coord,
                                                         Orientation orientation) const override;

        void PostLoadValidate(const data::PrototypeManager& proto) const override;

    private:
        /// Number of tiles which object occupies
        Dimension dimension_{1, 1};
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_H
