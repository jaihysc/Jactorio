#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "glm/glm.hpp"

namespace jactorio_renderer_mvp
{
	void set_mvp_uniform_location(int location);
	
	/*!
	 * Sends MVP data to GPU
	 */
	void update_shader_mvp();

	void setg_model_matrix(const glm::mat4& matrix);
	void setg_view_matrix(glm::mat4 matrix);
	void setg_projection_matrix(glm::mat4 matrix);
}

#endif // SHADER_MANAGER_H