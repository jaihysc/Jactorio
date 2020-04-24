// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_MVP_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_MVP_MANAGER_H
#pragma once

#include <glm/glm.hpp>

namespace jactorio::renderer
{
	// Functions for the Model, View and Projection matrices
	// VIEW

	/**
	 * Modifying the returned pointer will change the location of the camera
	 * @return pointer to camera transform vector
	 */
	glm::vec3* get_view_transform();

	/**
	 * Sets the current view transform
	 */
	void update_view_transform();


	// PROJECTION

	struct Projection_tile_data
	{
		unsigned short tiles_x;
		unsigned short tiles_y;
	};

	/**
	 * Converts Projection_tile_data from projection_calculate_tile_properties into a matrix <br>
	 * Guarantees a zoom of minimum of offset on all axis <br>
	 * May offset more in a certain axis to preserve aspect ratio
	 * @param window_width width of display area in pixels
	 * @param window_height Height of display area in pixels
	 * @param offset number of tiles horizontally to from the edge to hide (giving a zoom effect) <br>
	 * Vertical tiles is calculated based on ration of tiles in tile_data
	 */
	glm::mat4 to_proj_matrix(unsigned short window_width,
	                         unsigned short window_height, float offset);
};

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_MVP_MANAGER_H
