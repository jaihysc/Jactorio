// Functions for the Model, View and Projection matrices

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/manager/mvp_manager.h"

#include "renderer/manager/shader_manager.h"

// VIEW

glm::vec3 camera_transform = glm::vec3(0, 0, 0);

glm::vec3* jactorio_renderer_mvp::get_camera_transform() {
	return &camera_transform;
}

void jactorio_renderer_mvp::update_camera_transform() {
	const glm::mat4 view_mat = glm::translate(glm::mat4(1.f), camera_transform);
	setg_view_matrix(view_mat);
}


// PROJECTION

// Calculates number of tiles to draw X and Y
// Changed by camera_manager to modify zoom

unsigned int tile_count_x;
unsigned int tile_count_y;

unsigned int tile_width;

unsigned int jactorio_renderer_mvp::get_max_tile_count_x() {
	return tile_count_x;
}

unsigned int jactorio_renderer_mvp::get_max_tile_count_y() {
	return tile_count_y;
}

void jactorio_renderer_mvp::set_proj_calculation_tile_width(const unsigned int width) {
	tile_width = width;
}

void jactorio_renderer_mvp::calculate_tile_properties() {
	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	// Add 1 in case of decimal and end was truncated
	tile_count_x = m_viewport[2] / tile_width + 1;
	tile_count_y = m_viewport[3] / tile_width + 1;
}

glm::mat4 jactorio_renderer_mvp::get_proj_matrix() {
	return glm::ortho(
		0.f, static_cast<float>(tile_count_x),
		static_cast<float>(tile_count_y), 0.f,
		-1.f, 1.f);
}
