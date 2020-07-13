// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#pragma once

#include <memory>
#include <utility>

#include "core/data_type.h"
#include "data/prototype/entity/health_entity.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_segment.h"

namespace jactorio::data
{
	///
	/// \brief TransportLineData with a segment index of 0 manages a segment and will delete it when it is deleted
	struct TransportLineData : HealthEntityData
	{
		explicit TransportLineData(std::shared_ptr<game::TransportSegment> line_segment)
			: lineSegment(std::move(line_segment)) {
		}

		TransportLineData(const TransportLineData& other)     = delete;
		TransportLineData(TransportLineData&& other) noexcept = delete;

		///
		/// <Entry direction>_<Exit direction>
		enum class LineOrientation
		{
			// Following the layout of the sprite
			up_left = 10,
			up = 2,
			up_right = 8,

			right_up = 6,
			right = 0,
			right_down = 11,

			down_right = 5,
			down = 3,
			down_left = 7,

			left_down = 9,
			left = 1,
			left_up = 4,
		};

		/// The logic chunk line_segment associated
		std::shared_ptr<game::TransportSegment> lineSegment;

		/// The distance to the head of the transport line
		/// \remark For rendering purposes, the length should never exceed ~2 chunks at most
		uint8_t lineSegmentIndex = 0;

		LineOrientation orientation = LineOrientation::up;

		//

		///
		/// \brief Updates orientation and member set for rendering 
		void SetOrientation(LineOrientation orientation) {
			this->orientation = orientation;
			this->set         = static_cast<uint16_t>(orientation);
		}

		///
		/// \brief Converts lineOrientation to placementOrientation
		static Orientation ToOrientation(LineOrientation line_orientation);

		void OnDrawUniqueData(renderer::RendererLayer& layer,
		                      float x_offset, float y_offset) const override;
	};


	///
	/// \brief Abstract class for all everything which moves items (belts, underground belts, splitters)
	class TransportLine : public HealthEntity
	{
	protected:
		TransportLine() = default;

	public:
		///
		/// \brief Number of tiles traveled by each item on the belt per tick
		/// \remark For Python API use only
		PYTHON_PROP_I(TransportLine, double, speedFloat, 0.01f);

		/// Number of tiles traveled by each item on the belt per tick
		game::TransportLineOffset speed;


		// ======================================================================
		// Data access

		///
		/// \brief Attempts to retrieve transport line data at world coordinates on tile
		/// \return pointer to data or nullptr if non existent
		J_NODISCARD static TransportLineData* GetLineData(const game::WorldData& world_data,
		                                                  game::WorldData::WorldCoord world_x,
		                                                  game::WorldData::WorldCoord world_y);

		static TransportLineData::LineOrientation GetLineOrientation(Orientation orientation,
		                                                             TransportLineData* up,
		                                                             TransportLineData* right,
		                                                             TransportLineData* down,
		                                                             TransportLineData* left);

		///
		/// \brief Gets transport segment at world coords
		/// \return nullptr if no segment exists
		static std::shared_ptr<game::TransportSegment>* GetTransportSegment(game::WorldData& world_data,
		                                                                    game::WorldData::WorldCoord world_x,
		                                                                    game::WorldData::WorldCoord world_y);

		// ======================================================================
		// Game events

		void OnBuild(game::WorldData& world_data,
		             game::LogicData& logic_data,
		             const game::WorldData::WorldPair& world_coords,
		             game::ChunkTileLayer& tile_layer, Orientation orientation) const override;

		void OnNeighborUpdate(game::WorldData& world_data,
		                      game::LogicData& logic_data,
		                      const game::WorldData::WorldPair& emit_world_coords,
		                      const game::WorldData::WorldPair& receive_world_coords, Orientation emit_orientation) const override;

		void OnRemove(game::WorldData& world_data,
		              game::LogicData& logic_data,
		              const game::WorldData::WorldPair& world_coords, game::ChunkTileLayer& tile_layer) const override;


		Sprite::SetT OnRGetSet(Orientation orientation,
		                       game::WorldData& world_data,
		                       const game::WorldData::WorldPair& world_coords) const override;

		std::pair<Sprite*, Sprite::FrameT> OnRGetSprite(const UniqueDataBase* unique_data,
		                                                GameTickT game_tick) const override;;

		// ======================================================================
		// Data events
		void PostLoad() override {
			// Convert floating point speed to fixed precision decimal speed
			speed = game::TransportLineOffset(speedFloat);
		}

		void PostLoadValidate(const PrototypeManager& data_manager) const override {
			J_DATA_ASSERT(speedFloat >= 0.001, "Transport line speed below minimum 0.001");
			// Cannot exceed item_width because of limitations in the logic
			J_DATA_ASSERT(speedFloat < 0.25, "Transport line speed equal or above maximum of 0.25");
		}

		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
