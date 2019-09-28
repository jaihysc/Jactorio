#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/renderers.h"

#include "renderer/manager/shader_manager.h"

#include "renderer/opengl/error.h"


void jactorio_renderer::Renderer::draw(const jactorio_renderer_gl::Vertex_array& va, const jactorio_renderer_gl::Index_buffer& ib, const glm::vec3 transform) {
	va.bind();
	ib.bind();

	const glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), transform);
	setg_model_matrix(model_matrix);
	update_shader_mvp();

	DEBUG_OPENGL_CALL(glDrawElements(GL_TRIANGLES, ib.count(), GL_UNSIGNED_INT, nullptr));  // Pointer not needed as buffer is already bound
}

void jactorio_renderer::Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}