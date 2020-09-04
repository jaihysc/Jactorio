// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "renderer/opengl/error.h"
#include "renderer/opengl/vertex_array.h"

jactorio::renderer::VertexArray::VertexArray() {
    DEBUG_OPENGL_CALL(glGenVertexArrays(1, &id_));
    DEBUG_OPENGL_CALL(glBindVertexArray(id_));
}

jactorio::renderer::VertexArray::~VertexArray() {
    Unbind();
    DEBUG_OPENGL_CALL(glDeleteVertexArrays(1, &id_));
}

void jactorio::renderer::VertexArray::AddBuffer(const VertexBuffer* vb,
                                                const unsigned span,
                                                const unsigned int location) const {
    this->Bind();
    vb->Bind();

    // location here is referenced by the shader
    DEBUG_OPENGL_CALL(glEnableVertexAttribArray(location));
    DEBUG_OPENGL_CALL(glVertexAttribPointer(
        location, span, GL_FLOAT, GL_FALSE, sizeof(float) * span, static_cast<const void*>(nullptr)));
}

void jactorio::renderer::VertexArray::Bind() const {
    DEBUG_OPENGL_CALL(glBindVertexArray(id_));
}

void jactorio::renderer::VertexArray::Unbind() {
    DEBUG_OPENGL_CALL(glBindVertexArray(0));
}
