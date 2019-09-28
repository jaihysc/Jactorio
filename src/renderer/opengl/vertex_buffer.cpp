#include <GL/glew.h>

#include "renderer/opengl/error.h"
#include "renderer/opengl/vertex_buffer.h"

jactorio_renderer_gl::Vertex_buffer::Vertex_buffer(const void* data, const unsigned size) {
	// Create buffer
	DEBUG_OPENGL_CALL(glGenBuffers(1, &id_));

	// Load data into buffer
	bind();
	DEBUG_OPENGL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

jactorio_renderer_gl::Vertex_buffer::~Vertex_buffer() {
	DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
};

void jactorio_renderer_gl::Vertex_buffer::bind() const {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id_));

}

void jactorio_renderer_gl::Vertex_buffer::unbind() {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
