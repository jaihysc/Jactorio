// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#define JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#pragma once

#include <cstdint>
#include <utility>

#include "jactorio.h"
#include "data/prototype/type.h"
#include "game/world/chunk.h"

namespace jactorio
{
	namespace data
	{
		class Entity;
		class PrototypeManager;
	}

	namespace game
	{
		class PlayerData;
	}
}

namespace jactorio::game
{
	///
	/// \brief Handles mouse input and selection
	class MouseSelection
	{
		static constexpr OverlayLayer kCursorOverlayLayer = OverlayLayer::general;
		
	public:
		J_NODISCARD static double GetCursorX();
		J_NODISCARD static double GetCursorY();

		// ======================================================================
		// Client only mouse selection (affects only rendering) For Player mouse selection, see player_data

		///
		/// \brief Draws a selection box if NO entity is selected, otherwise, draws a ghost of the entity selected at the cursor
		void DrawCursorOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager);

		///
		/// \brief Draws selection box over entity & no item selected. | With item selected: draws ghost of entity
		void DrawOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager,
		                 const data::Entity* selected_entity, int world_x, int world_y,
		                 data::Orientation
		                 placement_orientation);
	private:
		ChunkCoord lastChunkPos_  = {0, 0};
		size_t lastOverlayElementIndex_ = UINT64_MAX;
	};

	///
	/// Callback provided to glfwSetCursorPosCallback to set mouse position
	void SetCursorPosition(double x_pos, double y_pos);
}

#endif //JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
