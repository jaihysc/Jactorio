#include <GL/glew.h>

#include "core/logger.h"

#include "renderer/opengl/texture.h"
#include "renderer/opengl/error.h"

unsigned int jactorio::renderer::Texture::bound_texture_id_ = 0;

jactorio::renderer::Texture::Texture(const data::Sprite* sprite)
	: renderer_id_(0), sprite_(sprite) {
	
	const unsigned char* texture_buffer = sprite->get_sprite_data_ptr();
	
	width_ = sprite->get_width();
	height_ = sprite->get_height();
	
	if (!texture_buffer) {
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
		width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer));

	// Rebind the last bound texture
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, bound_texture_id_));
}

jactorio::renderer::Texture::~Texture() {
	DEBUG_OPENGL_CALL(glDeleteTextures(1, &renderer_id_));
	delete sprite_;
}

void jactorio::renderer::Texture::bind(const unsigned int slot) const {
	// This can be dangerous, number of available slots unknown, TODO query openGL
	DEBUG_OPENGL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, renderer_id_));
	bound_texture_id_ = renderer_id_;
}

void jactorio::renderer::Texture::unbind() {
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
