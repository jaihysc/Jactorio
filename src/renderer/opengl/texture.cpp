// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <GL/glew.h>

#include <utility>

#include "renderer/opengl/texture.h"
#include "core/logger.h"
#include "renderer/renderer_exception.h"
#include "renderer/opengl/error.h"

unsigned int jactorio::renderer::Texture::boundTextureId_ = 0;

jactorio::renderer::Texture::Texture(std::shared_ptr<SpriteBufferT> buffer,
                                     const unsigned int width, const unsigned int height)
	: rendererId_(0), textureBuffer_(std::move(buffer)), width_(width), height_(height) {

	if (!textureBuffer_) {
		LOG_MESSAGE(error, "Received empty texture");
		return;
	}

	DEBUG_OPENGL_CALL(glGenTextures(1, &rendererId_));
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, rendererId_));

	// Required by openGL, handles when textures are too big/small
	DEBUG_OPENGL_CALL(
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	DEBUG_OPENGL_CALL(
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	DEBUG_OPENGL_CALL(
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	DEBUG_OPENGL_CALL(
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	DEBUG_OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
		width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureBuffer_.get()));

	// Rebind the last bound texture
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, boundTextureId_));
}

jactorio::renderer::Texture::~Texture() {
	DEBUG_OPENGL_CALL(glDeleteTextures(1, &rendererId_));
}

void jactorio::renderer::Texture::Bind(const unsigned int slot) const {
	// Ensure there is sufficient slots to bind the texture
	int texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	if (slot >= static_cast<unsigned int>(texture_units)) {
		LOG_MESSAGE_F(error,
		              "Texture slot out of bounds, attempting to bind at index %d", slot);
		throw RendererException("Texture slot out of bounds");
	}

	DEBUG_OPENGL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, rendererId_));
	boundTextureId_ = rendererId_;
}

void jactorio::renderer::Texture::Unbind() {
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
