// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_LABEL_H
#define JACTORIO_INCLUDE_PROTO_LABEL_H
#pragma once

#include "proto/framework/framework_base.h"

namespace jactorio::proto
{
    ///
    /// Constants for all labels' names
    struct LabelNames
    {
        /// Access each label by appending the number which corresponds to each PlayerAction::Type
        ///
        /// e.g: __core__/label:player-action-0
        ///      __core__/label:player-action-1
        ///      __core__/label:player-action-2
        static constexpr auto kPlayerActionPrefix = "__core__/label:player-action-";

        static constexpr auto kKeyActionNone     = "__core__/label:key-action-none";
        static constexpr auto kKeyActionKDown    = "__core__/label:key-action-key-down";
        static constexpr auto kKeyActionKPressed = "__core__/label:key-action-key-pressed";
        static constexpr auto kKeyActionKRepeat  = "__core__/label:key-action-key-repeat";
        static constexpr auto kKeyActionKHeld    = "__core__/label:key-action-key-held";
        static constexpr auto kKeyActionKUp      = "__core__/label:key-action-key-up";
    };

    ///
    /// Stores localized text
    class Label final : public FrameworkBase
    {
    public:
        PROTOTYPE_CATEGORY(label);

        void PostLoadValidate(const data::PrototypeManager& proto_manager) const override {}
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_LABEL_H
