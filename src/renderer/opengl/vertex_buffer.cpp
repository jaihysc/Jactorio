// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <GL/glew.h>

#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/error.h"

jactorio::renderer::VertexBuffer::VertexBuffer(const void* data, const uint32_t byte_size, const bool static_buffer) {
	DEBUG_OPENGL_CALL(glGenBuffers(1, &id_));

	Reserve(data, byte_size, static_buffer);
}

jactorio::renderer::VertexBuffer::~VertexBuffer() {
	Unbind();
	DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
};

// Buffering data

void* jactorio::renderer::VertexBuffer::Map() const {
	assert(id_ != 0);
	Bind();

	DEBUG_OPENGL_CALL(void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	return ptr;
}

void jactorio::renderer::VertexBuffer::UnMap() const {
	assert(id_ != 0);
	Bind();

	DEBUG_OPENGL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
}

/*
void jactorio::renderer::VertexBuffer::UpdateData(const void* data,
												  const uint32_t offset, const uint32_t size) const {
	Bind();
	DEBUG_OPENGL_CALL(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
}
*/

void jactorio::renderer::VertexBuffer::Reserve(const void* data, const uint32_t byte_size,
                                               const bool static_buffer) const {
	Bind();
	DEBUG_OPENGL_CALL(glBufferData(GL_ARRAY_BUFFER, byte_size, data,
		static_buffer ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
}

// Binding

void jactorio::renderer::VertexBuffer::Bind() const {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id_));

}

void jactorio::renderer::VertexBuffer::Unbind() {
	DEBUG_OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
