// 
// mvp_manager.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/14/2020
// 

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/mvp_manager.h"

#include "renderer/opengl/shader_manager.h"

// VIEW

glm::vec3 camera_transform = glm::vec3(0, 0, 0);

glm::vec3* jactorio::renderer::mvp_manager::get_view_transform() {
	return &camera_transform;
}

void jactorio::renderer::mvp_manager::update_view_transform() {
	const glm::mat4 view_mat = translate(glm::mat4(1.f), camera_transform);
	setg_view_matrix(view_mat);
}


// PROJECTION

// Calculates number of tiles to draw X and Y

jactorio::renderer::mvp_manager::Projection_tile_data jactorio::renderer::mvp_manager::
projection_calculate_tile_properties(const unsigned short tile_width,
                                     const unsigned short window_width,
                                     const unsigned short window_height) {
	Projection_tile_data tile_data{};

	tile_data.tiles_x = window_width / tile_width;
	tile_data.tiles_y = window_height / tile_width;

	return tile_data;
}

glm::mat4 jactorio::renderer::mvp_manager::to_proj_matrix(const unsigned short window_width,
                                                          const unsigned short window_height,
                                                          const float offset) {
	// Calculate aspect ratio scale based on "larger / smaller"
	float x_zoom_ratio = 1.f;
	float y_zoom_ratio = 1.f;
	if (window_width > window_height) {
		x_zoom_ratio = static_cast<float>(window_width) / static_cast<float>(window_height);
	}
	else {
		y_zoom_ratio = static_cast<float>(window_height) / static_cast<float>(window_width);
	}

	return glm::ortho(
		offset * x_zoom_ratio,
		static_cast<float>(window_width) - offset * x_zoom_ratio,

		static_cast<float>(window_height) - offset * y_zoom_ratio,
		offset * y_zoom_ratio,

		-1.f, 1.f);
}
