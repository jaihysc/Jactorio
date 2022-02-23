// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include <utility>

#include "core/convert.h"
#include "core/logger.h"
#include "render/opengl/error.h"
#include "render/opengl/texture.h"
#include "render/renderer_exception.h"

using namespace jactorio;

render::Texture::Texture(std::shared_ptr<SpriteBufferT> buffer, const DimensionT width, const DimensionT height)
    : id_(0), textureBuffer_(std::move(buffer)), width_(width), height_(height) {

    if (!textureBuffer_) {
        LOG_MESSAGE(error, "Received empty texture");
        return;
    }

    DEBUG_OPENGL_CALL(glGenTextures(1, &id_));
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, id_));

    // Required by openGL, handles when textures are too big/small
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    DEBUG_OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D,
                                   0,
                                   GL_RGBA8,
                                   SafeCast<GLsizei>(width),
                                   SafeCast<GLsizei>(height),
                                   0,
                                   GL_RGBA,
                                   GL_UNSIGNED_BYTE,
                                   textureBuffer_.get()));
}

render::Texture::~Texture() {
    // Opengl may not be setup when destructor called if exit early
    // Thus only make gl call if id_ non zero (know gl context exists)
    if (id_ != 0)
        DEBUG_OPENGL_CALL(glDeleteTextures(1, &id_));
}

render::Texture::Texture(Texture&& other) noexcept
    : id_{other.id_}, textureBuffer_{std::move(other.textureBuffer_)}, width_{other.width_}, height_{other.height_} {
    // Despite opengl silently accepting invalid ids, if a move is make, the old object deletes the texture!
    other.id_ = 0;
}

void render::Texture::Bind(const unsigned int slot) const {
    // Ensure there is sufficient slots to bind the texture
    int texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    if (slot >= SafeCast<unsigned int>(texture_units)) {
        LOG_MESSAGE_F(error, "Texture slot out of bounds, attempting to bind at index %d", slot);
        throw RendererException("Texture slot out of bounds");
    }

    DEBUG_OPENGL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, id_));
}

void render::Texture::Unbind() {
    DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
