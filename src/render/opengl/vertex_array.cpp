// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/opengl/vertex_array.h"

#include <GL/glew.h>

#include "render/opengl/error.h"
#include "render/opengl/vertex_buffer.h"

using namespace jactorio;

render::VertexArray::~VertexArray() {
    Unbind();
    DEBUG_OPENGL_CALL(glDeleteVertexArrays(1, &id_));
}

void render::VertexArray::Init() noexcept {
    DEBUG_OPENGL_CALL(glGenVertexArrays(1, &id_));
}

void render::VertexArray::AddBuffer(const VertexBuffer* vb,
                                    const unsigned span,
                                    const unsigned int location) const noexcept {
    Bind();
    vb->Bind();

    // location here is referenced by the shader
    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(location));
    DEBUG_OPENGL_CALL(glVertexAttribIPointer(
        location, span, GL_UNSIGNED_SHORT, sizeof(ElementT) * span, static_cast<const void*>(nullptr)));
}

void render::VertexArray::Bind() const noexcept {
    DEBUG_OPENGL_CALL(glBindVertexArray(id_));
}

void render::VertexArray::Unbind() noexcept {
    DEBUG_OPENGL_CALL(glBindVertexArray(0));
}
