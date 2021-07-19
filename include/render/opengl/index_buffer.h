// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_INDEX_BUFFER_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_INDEX_BUFFER_H
#pragma once

#include "jactorio.h"

namespace jactorio::render
{
    /// \remark Lifetime of object must be in opengl context
    class IndexBuffer
    {
    public:
        IndexBuffer() = default;
        ~IndexBuffer();

        IndexBuffer(const IndexBuffer& other)     = delete;
        IndexBuffer(IndexBuffer&& other) noexcept = default;
        IndexBuffer& operator=(const IndexBuffer& other) = delete;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept = default;

        /// Generates buffer
        void Init() noexcept;

        /// Creates a new buffer of provided specifications
        void Reserve(const void* data, uint32_t index_count) noexcept;

        /// Gets pointer to begin modifying buffer data
        /// \remark Ensure buffer is bound
        J_NODISCARD void* Map() const noexcept;

        /// Call to finish modifying buffer data, provided pointer from Map now invalid
        /// \remark Ensure buffer is bound
        void UnMap() const noexcept;

        void Bind() const noexcept;
        static void Unbind() noexcept;

        J_NODISCARD unsigned int Count() const noexcept;

    private:
        unsigned int id_    = 0;
        unsigned int count_ = 0;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_INDEX_BUFFER_H
