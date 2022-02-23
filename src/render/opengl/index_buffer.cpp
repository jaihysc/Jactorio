// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "render/opengl/error.h"
#include "render/opengl/index_buffer.h"

using namespace jactorio;

render::IndexBuffer::~IndexBuffer() {
    // Opengl may not be setup when destructor called if exit early
    // Thus only make gl call if id_ non zero (know gl context exists)
    if (id_ != 0)
        DEBUG_OPENGL_CALL(glDeleteBuffers(1, &id_));
}

render::IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept : id_{other.id_} {
    other.id_ = 0;
}

void render::IndexBuffer::Init() noexcept {
    DEBUG_OPENGL_CALL(glGenBuffers(1, &id_));
}

void render::IndexBuffer::Reserve(const void* data, const uint32_t index_count) noexcept {
    Bind();
    DEBUG_OPENGL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), data, GL_STATIC_DRAW));
}

void* render::IndexBuffer::Map() const noexcept {
    assert(id_ != 0);
    DEBUG_OPENGL_CALL(void* ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
    return ptr;
}

void render::IndexBuffer::UnMap() const noexcept {
    assert(id_ != 0);
    DEBUG_OPENGL_CALL(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
}

void render::IndexBuffer::Bind() const noexcept {
    DEBUG_OPENGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_));
}

void render::IndexBuffer::Unbind() noexcept {
    DEBUG_OPENGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
