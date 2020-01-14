#include <GL/glew.h>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/error.h"

jactorio::renderer::Index_buffer::Index_buffer(const unsigned* data, unsigned count) {
	DEBUG_OPENGL_CALL(glGenBuffers(1, &id_));
	reserve(data, count);
}

jactorio::renderer::Index_buffer::~Index_buffer() {
	unbind();
	DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
}

void jactorio::renderer::Index_buffer::reserve(const void *data, uint32_t index_count) {
	bind();
	DEBUG_OPENGL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), data, GL_STATIC_DRAW));
	count_ = index_count;
}

void jactorio::renderer::Index_buffer::bind() const {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_));
}

void jactorio::renderer::Index_buffer::unbind() {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

unsigned jactorio::renderer::Index_buffer::count() const {
	return count_;
}
