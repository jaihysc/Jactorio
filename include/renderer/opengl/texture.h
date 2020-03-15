// 
// texture.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/15/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
#pragma once

#include "data/prototype/sprite.h"

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
		int width_, height_;
		const data::Sprite* sprite_;

	public:
		explicit Texture(const data::Sprite* sprite);

		~Texture();

		Texture(const Texture& other) = delete;

		Texture(Texture&& other) noexcept = delete;

		Texture& operator=(const Texture& other) = delete;

		Texture& operator=(Texture&& other) noexcept = delete;

		void bind(unsigned int slot = 0) const;

		static void unbind();

		J_NODISCARD int get_width() const { return width_; }

		J_NODISCARD int get_height() const { return height_; }

		J_NODISCARD unsigned int get_id() const { return renderer_id_; }
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
