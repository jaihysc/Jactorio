#ifndef RENDERER_OPENGL_SHADER_MANAGER_H
#define RENDERER_OPENGL_SHADER_MANAGER_H

#include <glm/glm.hpp>

namespace jactorio::renderer
{
	void set_mvp_uniform_location(int location);

	/**
	 * Sends MVP data to GPU
	 */
	void update_shader_mvp();
	const glm::mat4& get_mvp_matrix();

	void setg_model_matrix(const glm::mat4& matrix);
	void setg_view_matrix(glm::mat4 matrix);
	void setg_projection_matrix(glm::mat4 matrix);
}

#endif // RENDERER_OPENGL_SHADER_MANAGER_H
