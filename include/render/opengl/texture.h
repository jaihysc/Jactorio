// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_TEXTURE_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_TEXTURE_H
#pragma once

#include "jactorio.h"

#include <memory>
#include <utility>

namespace jactorio::render
{
    /// Manages a jactorio::proto::Sprite* and creates a texture for OpenGL
    /// This will delete the sprite* when the texture is deleted
    class Texture
    {
        using DimensionT = uint64_t;

    public:
        using SpriteBufferT = unsigned char;

        /// \param buffer new allocated buffer, will be deleted when texture is deleted (assumed to be RGBA)
        /// \param width Width of buffer image
        /// \param height Height of buffer image
        Texture(std::shared_ptr<SpriteBufferT> buffer, DimensionT width, DimensionT height);
        ~Texture();

        Texture(const Texture& other) = delete;
        Texture(Texture&& other) noexcept;

        Texture& operator=(Texture other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(Texture& lhs, Texture& rhs) noexcept {
            using std::swap;
            swap(lhs.id_, rhs.id_);
            swap(lhs.textureBuffer_, rhs.textureBuffer_);
            swap(lhs.width_, rhs.width_);
            swap(lhs.height_, rhs.height_);
        }

        /// \exception Renderer_exception Index out of bounds
        void Bind(unsigned int slot = 0) const;

        static void Unbind();

        J_NODISCARD DimensionT Width() const {
            return width_;
        }
        J_NODISCARD DimensionT Height() const {
            return height_;
        }

        J_NODISCARD unsigned int GetId() const {
            return id_;
        }

    private:
        unsigned int id_;

        // Image properties
        std::shared_ptr<SpriteBufferT> textureBuffer_;
        DimensionT width_, height_;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_TEXTURE_H
