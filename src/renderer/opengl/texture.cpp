#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb/stb_image.h>

#include "renderer/opengl/texture.h"
#include "renderer/opengl/error.h"

jactorio::renderer::Texture::Texture(const std::string& texture_filepath)
	: renderer_id_(0), width_(0), height_(0), bytes_per_pixel_(0),
	texture_filepath_(texture_filepath), texture_buffer_(nullptr)
{
	// openGL reads images backwards, bottom left is (0,0)
	stbi_set_flip_vertically_on_load(true);
	texture_buffer_ = stbi_load(
		texture_filepath.c_str(),
		&width_,
		&height_,
		&bytes_per_pixel_,
		4  // 4 desired channels for RGBA
	);  

	if (!texture_buffer_) {
		std::cout << "Failed to read texture at filepath: " << texture_filepath << "\n";
		return;
	}
	
	DEBUG_OPENGL_CALL(glGenTextures(1, &renderer_id_));
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, renderer_id_));

	// Required by openGL, handles when textures are too big/small
	DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	DEBUG_OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	DEBUG_OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
		width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer_));
	
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

	// Free buffer
	if (texture_buffer_)
		stbi_image_free(texture_buffer_);
}

jactorio::renderer::Texture::~Texture() {
	DEBUG_OPENGL_CALL(glDeleteTextures(1, &renderer_id_));
}

void jactorio::renderer::Texture::bind(const unsigned int slot) const {
	// This can be dangerous, number of available slots unknown, TODO query openGL
	DEBUG_OPENGL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, renderer_id_));
}

void jactorio::renderer::Texture::unbind() const {
	DEBUG_OPENGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
