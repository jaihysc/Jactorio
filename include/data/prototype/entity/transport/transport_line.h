// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#pragma once

#include <functional>
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
			up_left = 9,
			up = 17,
			up_right = 11,

			right_up = 13,
			right = 19,
			right_down = 8,

			down_right = 14,
			down = 16,
			down_left = 12,

			left_down = 10,
			left = 18,
			left_up = 15,
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
	private:
		static void RemoveFromLogic(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
		                            game::TransportSegment& line_segment);

		/// Up, right, down, left
		using LineData4Way = TransportLineData*[4];

		///
		///	\brief Updates the orientation of current and neighboring transport lines 
		static void UpdateNeighboringOrientation(const game::WorldData& world_data,
		                                         const game::WorldData::WorldPair& world_coords,
		                                         TransportLineData* t_center,
		                                         TransportLineData* c_right,
		                                         TransportLineData* b_center,
		                                         TransportLineData* c_left,
		                                         TransportLineData* center);

		// world_x, world_y is the location offset from the original provided as function parameter
		using UpdateFunc = std::function<
			void(game::WorldData& world_data,
			     int world_x,
			     int world_y,
			     game::TransportSegment::TerminationType termination_type)>;

		using UpdateSideOnlyFunc = std::function<
			void(game::WorldData& world_data,
			     int world_x,
			     int world_y,
			     Orientation direction,
			     game::TransportSegment::TerminationType termination_type)>;

		///
		/// \brief Calls func or side_only_func depending on the line_orientation, provides parameters on how neighboring lines
		/// should be modified.
		/// \remark This does not move across logic chunks and may make the position negative
		/// \param func Called when line orientation is bending for updating provided line segment
		/// \param side_only_func Called when line orientation is straight for updating provided line segment 
		static void UpdateNeighborLines(game::WorldData& world_data,
		                                int32_t world_x,
		                                int32_t world_y,
		                                TransportLineData::LineOrientation line_orientation,
		                                const UpdateFunc& func,
		                                const UpdateSideOnlyFunc& side_only_func);

		static void UpdateSegmentHead(game::WorldData& world_data,
		                              const game::WorldData::WorldPair& world_coords,
		                              LineData4Way& line_data,
		                              const std::shared_ptr<game::TransportSegment>& line_segment);

		/*
		 * Transport line grouping rules:
		 *
		 * < < < [1, 2, 3] - Direction [order];
		 * Line ahead:
		 *		- Extends length of transport line segment
		 *
		 * < < < [3, 2, 1]
		 * Line behind:
		 *		- Moves head of transport segment, shift leading item 1 tile back
		 *		
		 * < < < [1, 3, 2]
		 * Line ahead and behind:
		 *		- Behaves as line ahead
		 */

		///
		/// \brief Initializes line data and groups transport segments
		/// Sets the transport segment grouped / newly created with in tile_layer and returns it
		/// \return Created data for at tile_layer, was a new transport segment created
		TransportLineData* InitTransportSegment(game::WorldData& world_data,
		                                        const game::WorldData::WorldPair& world_coords,
		                                        Orientation orientation,
		                                        game::ChunkTileLayer& tile_layer,
		                                        LineData4Way& line_data) const;

		///
		/// \brief Updates neighboring segments after transport line is removed 
		/// \param world_coords Coords of transport line removed
		/// \param line_data Neighboring line segment
		/// \param target Removed line segment
		static void DisconnectTargetSegment(game::WorldData& world_data,
		                                    const game::WorldData::WorldPair& world_coords,
		                                    TransportLineData* target, TransportLineData* line_data);
	public:
		void OnBuild(game::WorldData& world_data,
		             const game::WorldData::WorldPair& world_coords,
		             game::ChunkTileLayer& tile_layer,
		             Orientation orientation) const override;

		void OnNeighborUpdate(game::WorldData& world_data,
		                      const game::WorldData::WorldPair& emit_world_coords,
		                      const game::WorldData::WorldPair& receive_world_coords,
		                      Orientation emit_orientation) const override;

		void OnRemove(game::WorldData& world_data,
		              const game::WorldData::WorldPair& world_coords,
		              game::ChunkTileLayer& tile_layer) const override;


		std::pair<uint16_t, uint16_t> MapPlacementOrientation(Orientation orientation,
		                                                      game::WorldData& world_data,
		                                                      const game::WorldData::WorldPair& world_coords)
		const override;


		std::pair<Sprite*, RenderableData::FrameT> OnRGetSprite(const UniqueDataBase* unique_data,
		                                                        const GameTickT game_tick) const override {
			return {this->sprite, game_tick % sprite->frames};
		};

		// ======================================================================
		// Data events
		void PostLoad() override {
			// Convert floating point speed to fixed precision decimal speed
			speed = game::TransportLineOffset(speedFloat);
		}

		void PostLoadValidate() const override {
			J_DATA_ASSERT(speedFloat > 0.001, "Transport line speed below minimum 0.001");
			// Cannot exceed item_width because of limitations in the logic
			J_DATA_ASSERT(speedFloat < 0.25, "Transport line speed equal or above maximum of 0.25");
		}

		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
		}

		void OnRShowGui(game::PlayerData& /*player_data*/, game::ChunkTileLayer* /*tile_layer*/) const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
