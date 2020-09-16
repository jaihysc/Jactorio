// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include <utility>

#include "core/convert.h"
#include "core/logger.h"
#include "render/opengl/error.h"
#include "render/opengl/texture.h"
#include "render/renderer_exception.h"

unsigned int jactorio::render::Texture::boundTextureId_ = 0;

jactorio::render::Texture::Texture(std::shared_ptr<SpriteBufferT> buffer,
                                   const DimensionT width,
                                   const DimensionT height)
    : rendererId_(0), textureBuffer_(std::move(buffer)), width_(width), height_(height) {

    if (!textureBuffer_) {
        LOG_MESSAGE(error, "Received empty texture");
        return;
    }

    DEBUG_OPENGL_CALL(glGenTextures(1, &rendererId_));
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, rendererId_));

    // Required by openGL, handles when textures are too big/small
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    DEBUG_OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D,
                                   0,
                                   GL_RGBA8,
                                   core::SafeCast<GLsizei>(width),
                                   core::SafeCast<GLsizei>(height),
                                   0,
                                   GL_RGBA,
                                   GL_UNSIGNED_BYTE,
                                   textureBuffer_.get()));

    // Rebind the last bound texture
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, boundTextureId_));
}

jactorio::render::Texture::~Texture() {
    DEBUG_OPENGL_CALL(glDeleteTextures(1, &rendererId_));
}

void jactorio::render::Texture::Bind(const unsigned int slot) const {
    // Ensure there is sufficient slots to bind the texture
    int texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    if (slot >= core::SafeCast<unsigned int>(texture_units)) {
        LOG_MESSAGE_F(error, "Texture slot out of bounds, attempting to bind at index %d", slot);
        throw RendererException("Texture slot out of bounds");
    }

    DEBUG_OPENGL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, rendererId_));
    boundTextureId_ = rendererId_;
}

void jactorio::render::Texture::Unbind() {
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
