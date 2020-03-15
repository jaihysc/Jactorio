// 
// mouse_selection.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/21/2019
// Last modified: 03/11/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#define JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
#pragma once

#include <utility>
#include <string>

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

	private:
		std::pair<int, int> mouse_selected_tile_;

		/// Player position on latest calculate_selected_tile() call
		float last_player_x_ = 0.f;
		float last_player_y_ = 0.f;

	public:
		///
		/// \brief Call on game tick to calculate the coordinates of mouse selected tile
		/// Avoids repetitive calculations
		/// \param player_x Position of player in the world
		/// \param player_y
		void calculate_selected_tile(float player_x, float player_y);


		///
		/// Gets the world X, Y of the tile the mouse is hovered over, computed by calculate_selected_tile(x, y)
		J_NODISCARD std::pair<int, int> get_mouse_tile_coords() const { return mouse_selected_tile_; }

		///
		/// \return true if selected tile is within placement range
		J_NODISCARD bool selected_tile_in_range() const;


		// ======================================================================

	private:
		/// The last tile cannot be stored as a pointer as it can be deleted if the world was regenerated
		std::pair<int, int> last_tile_pos_;
		/// Dimensions of last tile(s), e.g 3 x 4 for a multi tile
		std::pair<int, int> last_tile_dimensions_;

		bool clear_entity_placement_ghost_ = false;

		///
		/// \brief Draws a cursor over the tile currently selected
		void draw_selection_box(Player_data& player_data);

	public:
		///
		/// \brief Sets a sprite on the overlay layer specified internal name at the cursor
		void set_tile_at_cursor(Player_data& player_data, const std::string& iname,
		                        int offset_x = 0, int offset_y = 0);

		///
		/// \brief Draws a selection box if NO entity is selected, otherwise, draws a ghost of the entity selected at the cursor
		void draw_cursor_overlay(Player_data& player_data);
	};

	///
	/// Callback provided to glfwSetCursorPosCallback to set mouse position
	void set_cursor_position(double x_pos, double y_pos);
}

#endif //JACTORIO_INCLUDE_GAME_INPUT_MOUSE_SELECTION_H
