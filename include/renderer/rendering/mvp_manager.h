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

	///
	/// \brief Modifying the returned pointer will change the location of the camera
	/// \return pointer to camera transform vector
	glm::vec3* GetViewTransform();

	///
	/// \brief Sets the current view transform
	void UpdateViewTransform();


	// PROJECTION

	///
	/// \brief Converts provided parameters into a matrix, guarantees a zoom of minimum of offset on all axis <br>
	/// May offset more in a certain axis to preserve aspect ratio
	/// \param window_width width of display area in pixels
	/// \param window_height Height of display area in pixels
	/// \param offset number of tiles horizontally to from the edge to hide (giving a zoom effect) <br>
	/// Vertical tiles is calculated based on ration of tiles in tile_data
	glm::mat4 ToProjMatrix(unsigned short window_width,
	                       unsigned short window_height, float offset);
};

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_MVP_MANAGER_H
