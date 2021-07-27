// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_SPLITTER_H
#define JACTORIO_INCLUDE_PROTO_SPLITTER_H
#pragma once

#include "proto/abstract/conveyor.h"

namespace jactorio::proto
{
    struct SplitterData final : ConveyorData
    {
        /// Whether or not left and right items should be swapped to opposite side
        bool swap = false;

        // Left lane is inherited
        // Right lane below:
        // TODO This is slightly wasteful as right also defines health which is unused,
        // but the logic expects distinct conveyors

        ConveyorData right;

        CEREAL_SERIALIZE(archive) {
            archive(swap, right, cereal::base_class<ConveyorData>(this));
        }
    };

    class Splitter final : public Conveyor
    {
    public:
        PROTOTYPE_CATEGORY(splitter);

        J_NODISCARD SpriteTexCoordIndexT OnGetTexCoordId(const game::World& world,
                                                         const WorldCoord& coord,
                                                         Orientation orientation) const override;

        void OnBuild(game::World& world,
                     game::Logic& logic,
                     const WorldCoord& coord,
                     Orientation orientation) const override;

        void OnNeighborUpdate(game::World& world,
                              game::Logic& logic,
                              const WorldCoord& emit_coord,
                              const WorldCoord& receive_coord,
                              Orientation emit_orientation) const override;

        void OnRemove(game::World& world, game::Logic& logic, const WorldCoord& coord) const override;

        void PostLoad() override;
        void PostLoadValidate(const data::PrototypeManager& proto) const override;

    private:
        /// Gets the coordinate of the non top left side of the splitter
        /// \remark Assumes current tile is part of a splitter
        static WorldCoord GetNonTopLeftCoord(const game::World& world, const WorldCoord& coord, game::TileLayer tlayer);
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_SPLITTER_H
