// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_ABSTRACT_CONVEYOR_H
#define JACTORIO_INCLUDE_PROTO_ABSTRACT_CONVEYOR_H
#pragma once

#include <memory>

#include "core/data_type.h"
#include "game/logic/conveyor_struct.h"
#include "proto/abstract/health_entity.h"

#include <cereal/types/memory.hpp>

namespace jactorio::proto
{
    struct ConveyorData : HealthEntityData
    {
        ConveyorData() = default;

        explicit ConveyorData(std::shared_ptr<game::ConveyorStruct> line_segment)
            : structure(std::move(line_segment)) {}


        std::shared_ptr<game::ConveyorStruct> structure = nullptr;

        /// Tile distance to the head of the conveyor
        /// \remark For rendering purposes, the length should never exceed ~2 chunks at most
        uint8_t structIndex = 0;


        CEREAL_SERIALIZE(archive) {
            archive(structure, structIndex, cereal::base_class<HealthEntityData>(this));
        }
    };


    /// Abstract class for all everything which moves items (belts, underground belts, splitters)
    class Conveyor : public HealthEntity
    {
    protected:
        Conveyor() = default;

    public:
        /// Number of tiles traveled by each item on the belt per tick
        /// \remark For Python API use only
        PYTHON_PROP_I(ProtoFloatT, speedFloat, 0.01);

        /// Number of tiles traveled by each item on the belt per tick
        LineDistT speed;

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

        void OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const override;


        // ======================================================================
        // Data events

        void PostLoad() override;
        void PostLoadValidate(const data::PrototypeManager& proto) const override;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ABSTRACT_TRANSPORT_LINE_H
