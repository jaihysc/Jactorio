#include "renderer/opengl/shader_manager.h"

#include "core/logger.h"
#include "renderer/opengl/shader.h"

int mvp_uniform_location = -1;

// If true, the matrix will be calculated and stored upon calling update_shader_mvp() or get_matrix()
bool calculate_matrix = false;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

glm::mat4 mvp_matrix;

/*!
 * Uniform location for "u_model_view_projection_matrix"
 * Must be set before MVP can be updated
 */
void jactorio::renderer::set_mvp_uniform_location(const int location) {
	mvp_uniform_location = location;

	LOG_MESSAGE(debug, "MVP uniform location set")
}

/**
 * Calculates MVP matrix if calculate_matrix is true
 */
void calculate_mvp_matrix() {
	if (calculate_matrix) {
		// Multiply backwards: projection, view, model
		mvp_matrix = projection * view * model;
		calculate_matrix = false;
	}
}

/*!
 * Sends current mvp matrices to GPU
 */
void jactorio::renderer::update_shader_mvp() {
	calculate_mvp_matrix();
	Shader::set_uniform_mat_4f(mvp_uniform_location, mvp_matrix);
}

const glm::mat4& jactorio::renderer::get_mvp_matrix() {
	calculate_mvp_matrix();
	return mvp_matrix;
}

void jactorio::renderer::setg_model_matrix(const glm::mat4& matrix) {
	model = matrix;
	calculate_matrix = true;
}

void jactorio::renderer::setg_view_matrix(const glm::mat4& matrix) {
	view = matrix;
	calculate_matrix = true;
}

void jactorio::renderer::setg_projection_matrix(const glm::mat4& matrix) {
	projection = matrix;
	calculate_matrix = true;
}
