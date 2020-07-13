// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_WORLD_RENDERER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_WORLD_RENDERER_H
#pragma once

#include "game/world/world_data.h"
#include "renderer/rendering/renderer.h"

/// Renders items utilizing the grid system of the map
namespace jactorio::renderer
{
	///
	/// Moves the world to match player_position_ <br>
	/// This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used
	/// \param world_data World to render
	/// \param renderer The renderer on which the world is drawn
	/// \param player_x X Position of the player in tiles
	/// \param player_y Y Position of the player in tiles
	void RenderPlayerPosition(GameTickT game_tick,
							  const game::WorldData& world_data,
							  Renderer* renderer, float player_x, float player_y);
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_WORLD_RENDERER_H
