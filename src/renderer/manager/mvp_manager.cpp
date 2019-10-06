#include <glm/gtc/matrix_transform.hpp>

#include "renderer/manager/mvp_manager.h"

#include "renderer/manager/shader_manager.h"

// VIEW

glm::vec3 jactorio::renderer::Mvp_manager::camera_transform_ = glm::vec3(0, 0, 0);

glm::vec3* jactorio::renderer::Mvp_manager::get_view_transform() {
	return &camera_transform_;
}

void jactorio::renderer::Mvp_manager::update_view_transform() {
	const glm::mat4 view_mat = glm::translate(glm::mat4(1.f), camera_transform_);
	setg_view_matrix(view_mat);
}


// PROJECTION

// Calculates number of tiles to draw X and Y
// Changed by camera_manager to modify zoom


jactorio::renderer::Mvp_manager::Projection_tile_data
	jactorio::renderer::Mvp_manager::projection_calculate_tile_properties(const unsigned short tile_width,
	                                                                      const unsigned short window_width,
																	      const unsigned short window_height) {
	Projection_tile_data tile_data;
	
	tile_data.tiles_x = window_width / tile_width;
	tile_data.tiles_y = window_height / tile_width;

	return tile_data;
}

glm::mat4 jactorio::renderer::Mvp_manager::to_proj_matrix(const Projection_tile_data tile_data) {
	return glm::ortho(
		0.f, static_cast<float>(tile_data.tiles_x),
		static_cast<float>(tile_data.tiles_y), 0.f,
		-1.f, 1.f);
}
