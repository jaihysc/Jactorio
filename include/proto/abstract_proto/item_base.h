// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_ABSTRACT_PROTO_ITEM_BASE_H
#define JACTORIO_INCLUDE_PROTO_ABSTRACT_PROTO_ITEM_BASE_H
#pragma once

#include "proto/framework/framework_base.h"

namespace jactorio::proto
{
    class Sprite;

    ///
    /// Basic abstract class for items
    class ItemBase : public FrameworkBase
    {
    protected:
        ItemBase() = default;

        explicit ItemBase(Sprite* sprite) : sprite(sprite) {}

    public:
        PYTHON_PROP_I(Sprite*, sprite, nullptr);

        void PostLoadValidate(const data::PrototypeManager& proto_manager) const override;
    };

    inline void ItemBase::PostLoadValidate(const data::PrototypeManager& /*proto_manager*/) const {
        J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified");
    }
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ABSTRACT_PROTO_ITEM_BASE_H
