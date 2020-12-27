// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_SPLITTER_H
#define JACTORIO_INCLUDE_PROTO_SPLITTER_H
#pragma once

#include "proto/abstract/conveyor.h"

namespace jactorio::proto
{
    struct SplitterData final : HealthEntityData
    {
        // HealthData of left and right will be unused

        ConveyorData left;
        ConveyorData right;
    };

    class Splitter : public Conveyor
    {
        PROTOTYPE_CATEGORY(splitter);

        void PostLoad() override;
        void PostLoadValidate(const data::PrototypeManager& proto_manager) const override;
        void ValidatedPostLoad() override;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_SPLITTER_H
