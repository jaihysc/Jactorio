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
    ///
    /// ConveyorData with a segment index of 0 manages a segment and will delete it when it is deleted
    struct ConveyorData final : HealthEntityData
    {
        ConveyorData() = default;

        explicit ConveyorData(std::shared_ptr<game::ConveyorStruct> line_segment)
            : structure(std::move(line_segment)) {}

        ///
        /// Updates orientation and member set for rendering
        void SetOrientation(LineOrientation orientation) {
            this->lOrien = orientation;
            this->set    = static_cast<uint16_t>(orientation);
        }

        static Orientation ToOrientation(LineOrientation line_orientation);


        std::shared_ptr<game::ConveyorStruct> structure = nullptr;

        /// Tile distance to the head of the conveyor
        /// \remark For rendering purposes, the length should never exceed ~2 chunks at most
        uint8_t structIndex = 0;

        LineOrientation lOrien = LineOrientation::up;


        CEREAL_SERIALIZE(archive) {
            archive(structure, structIndex, lOrien, cereal::base_class<HealthEntityData>(this));
        }

        CEREAL_LOAD_CONSTRUCT(archive, construct, ConveyorData) {
            std::shared_ptr<game::ConveyorStruct> line_segment;
            archive(line_segment);
            construct(line_segment);

            archive(construct->structIndex, construct->lOrien, cereal::base_class<HealthEntityData>(construct.ptr()));
        }
    };


    ///
    /// Abstract class for all everything which moves items (belts, underground belts, splitters)
    class Conveyor : public HealthEntity
    {
    protected:
        Conveyor() = default;

    public:
        ///
        /// Number of tiles traveled by each item on the belt per tick
        /// \remark For Python API use only
        PYTHON_PROP_I(ProtoFloatT, speedFloat, 0.01);

        /// Number of tiles traveled by each item on the belt per tick
        LineDistT speed;


        // ======================================================================
        // Game events

        void OnRDrawUniqueData(render::RendererLayer& layer,
                               const SpriteUvCoordsT& uv_coords,
                               const core::Position2<float>& pixel_offset,
                               const UniqueDataBase* unique_data) const override;

        J_NODISCARD SpriteSetT OnRGetSpriteSet(Orientation orientation,
                                               game::WorldData& world_data,
                                               const WorldCoord& world_coords) const override;

        J_NODISCARD SpriteFrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data,
                                                   GameTickT game_tick) const override;


        void OnBuild(game::WorldData& world,
                     game::LogicData& logic,
                     const WorldCoord& coord,
                     game::ChunkTileLayer& tile_layer,
                     Orientation orientation) const override;

        void OnNeighborUpdate(game::WorldData& world,
                              game::LogicData& logic,
                              const WorldCoord& emit_world_coords,
                              const WorldCoord& receive_world_coords,
                              Orientation emit_orientation) const override;

        void OnRemove(game::WorldData& world,
                      game::LogicData& logic,
                      const WorldCoord& coord,
                      game::ChunkTileLayer& tile_layer) const override;

        void OnDeserialize(game::WorldData& world_data,
                           const WorldCoord& world_coord,
                           game::ChunkTileLayer& tile_layer) const override;


        // ======================================================================
        // Data events

        void PostLoad() override;
        void PostLoadValidate(const data::PrototypeManager& proto_manager) const override;
        void ValidatedPostLoad() override;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ABSTRACT_TRANSPORT_LINE_H
