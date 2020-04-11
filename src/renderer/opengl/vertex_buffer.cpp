// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <GL/glew.h>

#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/error.h"

jactorio::renderer::Vertex_buffer::Vertex_buffer(const void* data, uint32_t byte_size, bool static_buffer) {
	DEBUG_OPENGL_CALL(glGenBuffers(1, &id_));

	reserve(data, byte_size, static_buffer);
}

jactorio::renderer::Vertex_buffer::~Vertex_buffer() {
	unbind();
	DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
};

// Buffering data

void jactorio::renderer::Vertex_buffer::
update_data(const void* data, const uint32_t offset, const uint32_t size) const {
	bind();
	DEBUG_OPENGL_CALL(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
}

void jactorio::renderer::Vertex_buffer::reserve(const void* data, uint32_t byte_size, bool static_buffer) const {
	bind();
	DEBUG_OPENGL_CALL(glBufferData(GL_ARRAY_BUFFER, byte_size, data,
		static_buffer ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
}

// Binding

void jactorio::renderer::Vertex_buffer::bind() const {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id_));

}

void jactorio::renderer::Vertex_buffer::unbind() {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
