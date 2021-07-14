// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_BUFFER_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_BUFFER_H
#pragma once

#include <cstdint>

#include "jactorio.h"

namespace jactorio::render
{
    /// \remark Lifetime of object must be in opengl context
    class VertexBuffer
    {
    public:
        VertexBuffer() = default;
        ~VertexBuffer();

        VertexBuffer(const VertexBuffer& other)     = delete;
        VertexBuffer(VertexBuffer&& other) noexcept = default;
        VertexBuffer& operator=(const VertexBuffer& other) = delete;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept = default;

        /// Generates buffer
        void Init() noexcept;

        /// Gets pointer to begin modifying buffer data
        J_NODISCARD void* Map() const noexcept;

        /// Call to finish modifying buffer data, provided pointer from Map now invalid
        void UnMap() const noexcept;

        // void UpdateData(const void* data, uint32_t offset, uint32_t size) const;

        /// Creates a new buffer of provided specifications
        void Reserve(const void* data, uint32_t byte_size, bool static_buffer) const noexcept;

        void Bind() const noexcept;
        static void Unbind() noexcept;

    private:
        unsigned int id_ = 0;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_BUFFER_H
