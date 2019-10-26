#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/mvp_manager.h"

#include "renderer/opengl/shader_manager.h"

// VIEW

glm::vec3 camera_transform = glm::vec3(0, 0, 0);

glm::vec3* jactorio::renderer::mvp_manager::get_view_transform() {
	return &camera_transform;
}

void jactorio::renderer::mvp_manager::update_view_transform() {
	const glm::mat4 view_mat = glm::translate(glm::mat4(1.f), camera_transform);
	setg_view_matrix(view_mat);
}


// PROJECTION

// Calculates number of tiles to draw X and Y

jactorio::renderer::mvp_manager::Projection_tile_data
	jactorio::renderer::mvp_manager::projection_calculate_tile_properties(const unsigned short tile_width,
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
	// Calculate aspect ratio based zoom
	const float zoom_ratio = static_cast<float>(window_height) / static_cast<float>(window_width);
	const float y_offset = offset * zoom_ratio;
	
	return glm::ortho(
		offset, static_cast<float>(window_width) - offset,
		static_cast<float>(window_height) - y_offset, y_offset,
		-1.f, 1.f);
}
