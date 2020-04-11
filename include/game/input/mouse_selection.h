// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/21/2019

#ifndef JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#define JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#pragma once

#include <string>
#include <utility>


#include "data/prototype/entity/entity.h"
#include "jactorio.h"

#include "game/player/player_data.h"

namespace jactorio::game
{
	///
	/// \brief Handles mouse input and selection
	class Mouse_selection
	{
	public:
		J_NODISCARD static double get_cursor_x();
		J_NODISCARD static double get_cursor_y();

		// ======================================================================
		// Client only mouse selection (affects only rendering) For Player mouse selection, see player_data
	private:
		/// The last tile cannot be stored as a pointer as it can be deleted if the world was regenerated
		std::pair<int, int> last_tile_pos_{0, 0};
		/// Dimensions of last tile(s), e.g 3 x 4 for a multi tile
		std::pair<int, int> last_tile_dimensions_{1, 1};

	public:
		///
		/// \brief Draws a selection box if NO entity is selected, otherwise, draws a ghost of the entity selected at the cursor
		void draw_cursor_overlay(Player_data& player_data);

		///
		/// \brief Draws selection box over entity & no item selected. | With item selected: draws ghost of entity
		void draw_overlay(Player_data& player_data, data::Entity* selected_entity, int world_x, int world_y,
		                  data::placementOrientation
		                  placement_orientation);
	};

	///
	/// Callback provided to glfwSetCursorPosCallback to set mouse position
	void set_cursor_position(double x_pos, double y_pos);
}

#endif //JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
