// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "render/opengl/error.h"
#include "render/opengl/index_buffer.h"

jactorio::render::IndexBuffer::IndexBuffer(const unsigned* data, const unsigned count) {
    DEBUG_OPENGL_CALL(glGenBuffers(1, &id_));
    Reserve(data, count);
}

jactorio::render::IndexBuffer::~IndexBuffer() {
    Unbind();
    DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
}

void jactorio::render::IndexBuffer::Reserve(const void* data, const uint32_t index_count) {
    Bind();
    DEBUG_OPENGL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), data, GL_STATIC_DRAW));
    count_ = index_count;
}

void jactorio::render::IndexBuffer::Bind() const {
    DEBUG_OPENGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_));
}

void jactorio::render::IndexBuffer::Unbind() {
    DEBUG_OPENGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

unsigned jactorio::render::IndexBuffer::Count() const {
    return count_;
}
