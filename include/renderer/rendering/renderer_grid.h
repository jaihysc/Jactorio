// 
// renderer_grid.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/28/2020
// 

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_GRID_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_GRID_H
#pragma once

#include "jactorio.h"

#include "renderer/rendering/renderer_layer.h"

// Maintains the grid layout used by the renderer
// Examples include:
//		Tile grid based on screen resolution + zoom for rendering the world
//		Default Texture spritemap positions
//		Indices for tile and texture grid

namespace jactorio::renderer::renderer_grid
{
	///
	/// \brief Generates indices to draw tiles using the grid from gen_render_grid
	/// \returns Indices to be feed into Index_buffer
	unsigned int* gen_render_grid_indices(uint32_t tile_count);
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_GRID_H
