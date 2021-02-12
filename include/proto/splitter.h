// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_SPLITTER_H
#define JACTORIO_INCLUDE_PROTO_SPLITTER_H
#pragma once

#include "proto/abstract/conveyor.h"

namespace jactorio::proto
{
    struct SplitterData final : HealthEntityData
    {
        explicit SplitterData(const Orientation orien) : orientation(orien) {}


        // HealthData of left and right will be unused
        ConveyorData left;
        ConveyorData right;

        /// Orientation of this splitter
        Orientation orientation;
    };

    class Splitter : public Conveyor
    {
    public:
        PROTOTYPE_CATEGORY(splitter);


        void OnBuild(game::World& world,
                     game::Logic& logic,
                     const WorldCoord& coord,
                     game::TileLayer tlayer,
                     Orientation orientation) const override;

        void OnNeighborUpdate(game::World& world,
                              game::Logic& logic,
                              const WorldCoord& emit_coord,
                              const WorldCoord& receive_coord,
                              Orientation emit_orientation) const override;

        void OnRemove(game::World& world,
                      game::Logic& logic,
                      const WorldCoord& coord,
                      game::TileLayer tlayer) const override;

        void PostLoad() override;
        void PostLoadValidate(const data::PrototypeManager& proto) const override;
        void ValidatedPostLoad() override;

    private:
        ///
        /// Gets the coordinate of the non top left side of the splitter
        /// \remark Assumes current tile is part of a splitter
        static WorldCoord GetNonTopLeftCoord(const game::World& world, const WorldCoord& coord);
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_SPLITTER_H
