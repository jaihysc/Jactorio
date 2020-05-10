// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#pragma once

#include <functional>

#include "core/data_type.h"
#include "data/prototype/entity/health_entity.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_line_structure.h"

namespace jactorio::data
{
	struct TransportLineData : HealthEntityData
	{
		explicit TransportLineData(game::TransportLineSegment& line_segment)
			: lineSegment(line_segment) {
		}

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
		std::reference_wrapper<game::TransportLineSegment> lineSegment;

		/// The distance to the head of the transport line
		/// \remark For rendering purposes, the length should never exceed ~2 chunks at most
		uint8_t lineSegmentIndex = 0;

		LineOrientation orientation = LineOrientation::up;

		// ======================================================================
		// Methods

		///
		/// \brief Updates orientation and member set for rendering 
		void SetOrientation(LineOrientation orientation) {
			this->orientation = orientation;
			this->set         = static_cast<uint16_t>(orientation);
		}

		///
		/// \brief Converts lineOrientation to placementOrientation
		static Orientation ToOrientation(const LineOrientation line_orientation) {
			switch (line_orientation) {
			case LineOrientation::up:
			case LineOrientation::right_up:
			case LineOrientation::left_up:
				return Orientation::up;

			case LineOrientation::right:
			case LineOrientation::up_right:
			case LineOrientation::down_right:
				return Orientation::right;

			case LineOrientation::down:
			case LineOrientation::right_down:
			case LineOrientation::left_down:
				return Orientation::down;

			case LineOrientation::left:
			case LineOrientation::up_left:
			case LineOrientation::down_left:
				return Orientation::left;

			default:
				assert(false);  // Missing switch case
				return Orientation::up;
			}
		}
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
		/// \brief Attempts to find transport line at world_x, world_y
		/// \param callback Called for each Chunk_struct_layer found matching Transport_line_data at world_x, world_y
		/// \return Logic_chunk if it exists, otherwise nullptr
		static void GetLineStructLayer(game::WorldData& world_data,
		                               game::WorldData::WorldCoord world_x,
		                               game::WorldData::WorldCoord world_y,
		                               const std::function<void(game::ChunkStructLayer&, game::LogicChunk&)>& callback);
		// ======================================================================
		// Game events
	private:
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

		using UpdateFunc = std::function<
			void(game::WorldData& world_data,
			     int world_x,
			     int world_y,
			     float world_offset_x,
			     float world_offset_y,
			     game::TransportLineSegment::TerminationType termination_type)>;

		using UpdateSideOnlyFunc = std::function<
			void(game::WorldData& world_data,
			     int world_x,
			     int world_y,
			     float world_offset_x,
			     float world_offset_y,
			     Orientation direction,
			     game::TransportLineSegment::TerminationType termination_type)>;

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
		                              game::TransportLineSegment& line_segment);

		///
		/// \brief Updates the world tiles which references a transport segment, props: line_segment_index, line_segment
		/// \param world_coords Beginning tile to update
		/// \param line_segment Beginning segment, traveling inverse Orientation line_segment.length tiles, <br>
		/// all tiles set to reference this
		/// \param offset Offsets segment id numbering, world_coords must be also adjusted to the appropriate offset when calling
		static void UpdateSegmentTiles(const game::WorldData& world_data,
		                               const game::WorldData::WorldPair& world_coords,
		                               game::TransportLineSegment& line_segment,
		                               int offset = 0);

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
		                                    TransportLineData* line_data, TransportLineData* target);
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


		std::pair<Sprite*, RenderableData::frame_t> OnRGetSprite(UniqueDataBase* unique_data,
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

		void OnRShowGui(game::PlayerData& /*player_data*/, game::ChunkTileLayer* /*tile_layer*/) const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
