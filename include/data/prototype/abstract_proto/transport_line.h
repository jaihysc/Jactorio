// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#pragma once

#include <memory>

#include "core/data_type.h"
#include "data/prototype/abstract_proto/health_entity.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_segment.h"
#include "render/rendering/renderer.h"

#include <cereal/types/memory.hpp>

namespace jactorio::data
{
    ///
    /// TransportLineData with a segment index of 0 manages a segment and will delete it when it is deleted
    struct TransportLineData final : HealthEntityData
    {
        explicit TransportLineData(std::shared_ptr<game::TransportSegment> line_segment)
            : lineSegment(std::move(line_segment)) {}

        ///
        /// <Entry direction>_<Exit direction>
        enum class LineOrientation
        {
            // Following the layout of the sprite
            up_left  = 10,
            up       = 2,
            up_right = 8,

            right_up   = 6,
            right      = 0,
            right_down = 11,

            down_right = 5,
            down       = 3,
            down_left  = 7,

            left_down = 9,
            left      = 1,
            left_up   = 4,
        };

        ///
        /// Updates orientation and member set for rendering
        void SetOrientation(LineOrientation orientation) {
            this->orientation = orientation;
            this->set         = static_cast<uint16_t>(orientation);
        }

        static Orientation ToOrientation(LineOrientation line_orientation);


        //


        /// The logic chunk line_segment associated
        std::shared_ptr<game::TransportSegment> lineSegment;

        /// Tile distance to the head of the transport line
        /// \remark For rendering purposes, the length should never exceed ~2 chunks at most
        uint8_t lineSegmentIndex = 0;

        LineOrientation orientation = LineOrientation::up;


        CEREAL_SERIALIZE(archive) {
            archive(lineSegment, lineSegmentIndex, orientation, cereal::base_class<HealthEntityData>(this));
        }

        CEREAL_LOAD_CONSTRUCT(archive, construct, TransportLineData) {
            std::shared_ptr<game::TransportSegment> line_segment;
            archive(line_segment);
            construct(line_segment);

            archive(construct->lineSegmentIndex,
                    construct->orientation,
                    cereal::base_class<HealthEntityData>(construct.ptr()));
        }
    };


    ///
    /// Abstract class for all everything which moves items (belts, underground belts, splitters)
    class TransportLine : public HealthEntity
    {
    protected:
        TransportLine() = default;

    public:
        /// up, right, down, left
        using LineData4Way = std::array<TransportLineData*, 4>;

        ///
        /// Number of tiles traveled by each item on the belt per tick
        /// \remark For Python API use only
        PYTHON_PROP_I(ProtoFloatT, speedFloat, 0.01);

        /// Number of tiles traveled by each item on the belt per tick
        LineDistT speed;


        // ======================================================================
        // Data access

        ///
        /// Attempts to retrieve transport line data at world coordinates on tile
        /// \return pointer to data or nullptr if non existent
        J_NODISCARD static TransportLineData* GetLineData(game::WorldData& world_data,
                                                          WorldCoordAxis world_x,
                                                          WorldCoordAxis world_y);

        J_NODISCARD static const TransportLineData* GetLineData(const game::WorldData& world_data,
                                                                WorldCoordAxis world_x,
                                                                WorldCoordAxis world_y);

        ///
        /// Gets line data for the 4 neighbors of origin coord
        J_NODISCARD static LineData4Way GetLineData4(game::WorldData& world_data, const WorldCoord& origin_coord);

        ///
        /// Gets transport segment at world coords
        /// \return nullptr if no segment exists
        static std::shared_ptr<game::TransportSegment>* GetTransportSegment(game::WorldData& world_data,
                                                                            WorldCoordAxis world_x,
                                                                            WorldCoordAxis world_y);

        ///
        /// Determines line orientation given orientation and neighbors
        static TransportLineData::LineOrientation GetLineOrientation(Orientation orientation,
                                                                     const LineData4Way& line_data4);


        // ======================================================================
        // Game events

        void OnRDrawUniqueData(render::RendererLayer& layer,
                               const SpriteUvCoordsT& uv_coords,
                               const core::Position2<float>& pixel_offset,
                               const UniqueDataBase* unique_data) const override;

        J_NODISCARD Sprite::SetT OnRGetSpriteSet(Orientation orientation,
                                                 game::WorldData& world_data,
                                                 const WorldCoord& world_coords) const override;

        J_NODISCARD Sprite::FrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data,
                                                     GameTickT game_tick) const override;


        void OnBuild(game::WorldData& world_data,
                     game::LogicData& logic_data,
                     const WorldCoord& world_coords,
                     game::ChunkTileLayer& tile_layer,
                     Orientation orientation) const override;

        void OnNeighborUpdate(game::WorldData& world_data,
                              game::LogicData& logic_data,
                              const WorldCoord& emit_world_coords,
                              const WorldCoord& receive_world_coords,
                              Orientation emit_orientation) const override;

        void OnRemove(game::WorldData& world_data,
                      game::LogicData& logic_data,
                      const WorldCoord& world_coords,
                      game::ChunkTileLayer& tile_layer) const override;

        void OnDeserialize(game::WorldData& world_data,
                           const WorldCoord& world_coord,
                           game::ChunkTileLayer& tile_layer) const override;


        // ======================================================================
        // Data events
        void PostLoad() override {
            // Convert floating point speed to fixed precision decimal speed
            speed = LineDistT(speedFloat);
        }

        void PostLoadValidate(const PrototypeManager& /*proto_manager*/) const override {
            J_DATA_ASSERT(speedFloat >= 0.001, "Transport line speed below minimum 0.001");
            // Cannot exceed item_width because of limitations in the logic
            J_DATA_ASSERT(speedFloat < 0.25, "Transport line speed equal or above maximum of 0.25");
        }

        void ValidatedPostLoad() override {
            sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
        }
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
