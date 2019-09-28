#include "core/logger.h"

#include "renderer/manager/shader_manager.h"
#include "renderer/opengl/shader.h"

int mvp_uniform_location = -1;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

/*!
 * Uniform location for "u_model_view_projection_matrix"
 * Must be set before MVP can be updated
 */
void jactorio_renderer::set_mvp_uniform_location(int location) {
	mvp_uniform_location = location;

	logger::log_message(logger::debug, "Shaders", "mvp uniform location set");
}

/*!
 * Sends current mvp matrices to GPU
 */
void jactorio_renderer::update_shader_mvp() {
	// Multiply backwards: projection, view, model
	glm::mat4 mvp = projection * view * model;
	jactorio_renderer_gl::Shader::set_uniform_mat_4f(mvp_uniform_location, mvp);
}

void jactorio_renderer::setg_model_matrix(const glm::mat4& matrix) {
	model = matrix;
}

void jactorio_renderer::setg_view_matrix(const glm::mat4 matrix) {
	view = matrix;
}

void jactorio_renderer::setg_projection_matrix(const glm::mat4 matrix) {
	projection = matrix;
}
