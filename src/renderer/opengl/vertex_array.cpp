#include <GL/glew.h>

#include "renderer/opengl/vertex_array.h"
#include "renderer/opengl/error.h"

jactorio::renderer::Vertex_array::Vertex_array() {
	DEBUG_OPENGL_CALL(glGenVertexArrays(1, &id_));
	DEBUG_OPENGL_CALL(glBindVertexArray(id_));
}

jactorio::renderer::Vertex_array::~Vertex_array() {
	DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
}

void jactorio::renderer::Vertex_array::add_buffer(const Vertex_buffer& vb, const unsigned span, const unsigned int location) const{
	vb.bind();

	// location here is referenced by the shader
	DEBUG_OPENGL_CALL(glEnableVertexAttribArray(location));
	DEBUG_OPENGL_CALL(glVertexAttribPointer(location, span, GL_FLOAT, GL_FALSE, sizeof(float) * span, static_cast<const void*>(nullptr)));
}

void jactorio::renderer::Vertex_array::bind() const {
	DEBUG_OPENGL_CALL(glBindVertexArray(id_));
}

void jactorio::renderer::Vertex_array::unbind() {
	DEBUG_OPENGL_CALL(glBindVertexArray(0));
}
