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
        /// Access each label by appending a number of each PlayerAction::Type
        ///
        /// e.g: label_player-action-0
        ///      label_player-action-1
        ///      label_player-action-2
        static constexpr auto kPlayerActionPrefix = "__core__/label_player-action-";
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
