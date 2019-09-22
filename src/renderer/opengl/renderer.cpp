#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/opengl/shader_manager.h"


#include "renderer/opengl/renderer.h"
#include "renderer/opengl/error.h"

Renderer::Renderer() {
	
}

Renderer::~Renderer() {
	
}

void Renderer::draw(const Vertex_array& va, const Index_buffer& ib, const glm::vec3 transform) const {
	va.bind();
	ib.bind();

	const glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), transform);
	jactorio::setg_model_matrix(model_matrix);
	jactorio::update_shader_mvp();

	DEBUG_OPENGL_CALL(glDrawElements(GL_TRIANGLES, ib.count(), GL_UNSIGNED_INT, nullptr));  // Pointer not needed as buffer is already bound
}

void Renderer::clear() const {
	glClear(GL_COLOR_BUFFER_BIT);
}
