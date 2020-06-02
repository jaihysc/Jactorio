// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
#pragma once

#include "jactorio.h"

#include <memory>

namespace jactorio::renderer
{
	///
	/// \brief Manages a jactorio::data::Sprite* and creates a texture for OpenGL <br>
	/// This will delete the sprite* when the texture is deleted
	class Texture
	{
	public:
		using SpriteBufferT = unsigned char;

		///
		/// \param buffer new allocated buffer, will be deleted when texture is deleted (assumed to be RGBA)
		/// \param width Width of buffer image
		/// \param height Height of buffer image
		Texture(std::shared_ptr<SpriteBufferT> buffer, unsigned int width, unsigned height);
		~Texture();

		Texture(const Texture& other)                = delete;
		Texture(Texture&& other) noexcept            = delete;
		Texture& operator=(const Texture& other)     = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

		///
		/// \exception Renderer_exception Index out of bounds
		void Bind(unsigned int slot = 0) const;

		static void Unbind();

		J_NODISCARD int Width() const { return width_; }
		J_NODISCARD int Height() const { return height_; }

		J_NODISCARD unsigned int GetId() const { return rendererId_; }

	private:
		static unsigned int boundTextureId_;
		unsigned int rendererId_;

		// Image properties
		std::shared_ptr<SpriteBufferT> textureBuffer_;
		unsigned int width_, height_;
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_TEXTURE_H
