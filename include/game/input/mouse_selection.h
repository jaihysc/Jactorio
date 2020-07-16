// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#define JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#pragma once

#include <utility>

#include "jactorio.h"
#include "data/prototype/type.h"

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
	public:
		J_NODISCARD static double GetCursorX();
		J_NODISCARD static double GetCursorY();

		// ======================================================================
		// Client only mouse selection (affects only rendering) For Player mouse selection, see player_data
	private:
		/// The last tile cannot be stored as a pointer as it can be deleted if the world was regenerated
		std::pair<int, int> lastTilePos_{0, 0};
		/// Dimensions of last tile(s), e.g 3 x 4 for a multi tile
		std::pair<int, int> lastTileDimensions_{1, 1};

	public:
		///
		/// \brief Draws a selection box if NO entity is selected, otherwise, draws a ghost of the entity selected at the cursor
		void DrawCursorOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager);

		///
		/// \brief Draws selection box over entity & no item selected. | With item selected: draws ghost of entity
		void DrawOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager,
		                 data::Entity* selected_entity, int world_x, int world_y,
		                 data::Orientation
		                 placement_orientation);
	};

	///
	/// Callback provided to glfwSetCursorPosCallback to set mouse position
	void SetCursorPosition(double x_pos, double y_pos);
}

#endif //JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
