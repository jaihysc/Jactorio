// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_WORLD_RENDERER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_WORLD_RENDERER_H
#pragma once

#include "game/world/logic_chunk.h"
#include "game/world/world_data.h"
#include "renderer/rendering/renderer.h"

/// Renders items utilizing the grid system of the map
namespace jactorio::renderer::world_renderer
{
	///
	/// \brief Draws chunks to the screen
	/// Attempting to draw chunks which do not exist will result in the chunk being queued for generation
	/// \param layer_index Index of layer to render
	/// \param is_tile_layer Is the provided layer_index for a tile layer? If false is object layer
	/// \param render_offset_x Tiles from window left to offset rendering
	/// \param render_offset_y Tiles from window top to offset rendering
	/// \param chunk_start_x Chunk to begin rendering
	/// \param chunk_start_y Chunk to begin rendering
	/// \param chunk_amount_x Number of chunks on X axis after chunk_start_x to draw
	/// \param chunk_amount_y Number of chunks on Y axis after chunk_start_y to draw
	/// \param layer Layer on which vertex and UV draw data will be placed
	void prepare_chunk_draw_data(const game::World_data& world_data,
	                             int layer_index, bool is_tile_layer,
	                             int render_offset_x, int render_offset_y,
	                             int chunk_start_x, int chunk_start_y,
	                             int chunk_amount_x, int chunk_amount_y,
	                             Renderer_layer* layer);

	///
	/// Prepares draw data for some of the structures in chunks (transport segments)
	/// \param l_chunk
	/// \param layer
	void prepare_logic_chunk_draw_data(game::Logic_chunk* l_chunk, Renderer_layer* layer);

	///
	/// Moves the world to match player_position_ <br>
	/// This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used
	/// \param world_data World to render
	/// \param renderer The renderer on which the world is drawn
	/// \param player_x X Position of the player in tiles
	/// \param player_y Y Position of the player in tiles
	void render_player_position(const game::World_data& world_data, Renderer* renderer, float player_x, float player_y);
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_WORLD_RENDERER_H
