// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <GL/glew.h>

#include "renderer/opengl/texture.h"
#include "core/logger.h"
#include "renderer/opengl/error.h"
#include "renderer/renderer_exception.h"

unsigned int jactorio::renderer::Texture::bound_texture_id_ = 0;

jactorio::renderer::Texture::Texture(unsigned char* buffer, unsigned int width, unsigned int height)
	: renderer_id_(0), texture_buffer_(buffer), width_(width), height_(height) {

	if (!texture_buffer_) {
		LOG_MESSAGE(error, "Received empty texture")
		return;
	}

	DEBUG_OPENGL_CALL(glGenTextures(1, &renderer_id_));
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, renderer_id_));

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
		width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer_));

	// Rebind the last bound texture
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, bound_texture_id_));
}

jactorio::renderer::Texture::~Texture() {
	DEBUG_OPENGL_CALL(glDeleteTextures(1, &renderer_id_));
	delete[] texture_buffer_;
}

void jactorio::renderer::Texture::bind(const unsigned int slot) const {
	// Ensure there is sufficient slots to bind the texture
	int texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	if (slot >= static_cast<unsigned int>(texture_units)) {
		LOG_MESSAGE_f(error,
		              "Texture slot out of bounds, attempting to bind at index %d", slot);
		throw Renderer_exception("Texture slot out of bounds");
	}

	DEBUG_OPENGL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, renderer_id_));
	bound_texture_id_ = renderer_id_;
}

void jactorio::renderer::Texture::unbind() {
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
