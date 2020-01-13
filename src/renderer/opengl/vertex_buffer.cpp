#include <GL/glew.h>

#include "renderer/opengl/error.h"
#include "renderer/opengl/vertex_buffer.h"

jactorio::renderer::Vertex_buffer::Vertex_buffer(const void* data, const unsigned size) {
	// Create buffer
	DEBUG_OPENGL_CALL(glGenBuffers(1, &id_));

	// Load data into buffer
	bind();
	DEBUG_OPENGL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

jactorio::renderer::Vertex_buffer::~Vertex_buffer() {
	unbind();
	DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
};

void jactorio::renderer::Vertex_buffer::set_buffer_data(const void* data, const unsigned offset, const unsigned size) const {
    bind();
	DEBUG_OPENGL_CALL(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
}

void jactorio::renderer::Vertex_buffer::bind() const {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id_));

}

void jactorio::renderer::Vertex_buffer::unbind() {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
