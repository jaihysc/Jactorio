// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
#pragma once

#include "jactorio.h"

namespace jactorio::renderer
{
	/**
	 * Manages a jactorio::data::Sprite* and creates a texture for OpenGL <br>
	 * This will delete the sprite* when the texture is deleted
	 */
	class Texture
	{
		static unsigned int bound_texture_id_;
		unsigned int renderer_id_;

		// Image properties
		unsigned char* texture_buffer_;
		unsigned int width_, height_;

	public:
		///
		/// \param buffer new allocated buffer, will be deleted when texture is deleted (assumed to be RGBA)
		/// \param width Width of buffer image
		/// \param height Height of buffer image
		Texture(unsigned char* buffer, unsigned int width, unsigned height);
		~Texture();

		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

		///
		/// \exception Renderer_exception Index out of bounds
		void bind(unsigned int slot = 0) const;

		static void unbind();

		J_NODISCARD int width() const { return width_; }
		J_NODISCARD int height() const { return height_; }

		J_NODISCARD unsigned int get_id() const { return renderer_id_; }
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
