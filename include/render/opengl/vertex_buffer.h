// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
#pragma once

#include <cstdint>

#include "jactorio.h"

namespace jactorio::render
{
    class VertexBuffer
    {
    public:
        VertexBuffer(const void* data, uint32_t byte_size, bool static_buffer);
        ~VertexBuffer();

        VertexBuffer(const VertexBuffer& other)     = delete;
        VertexBuffer(VertexBuffer&& other) noexcept = delete;
        VertexBuffer& operator=(const VertexBuffer& other) = delete;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept = delete;

        ///
        /// Gets pointer to begin modifying buffer data
        J_NODISCARD void* Map() const;

        ///
        /// Call to finish modifying buffer data, provided pointer from Map now invalid
        void UnMap() const;

        // void UpdateData(const void* data, uint32_t offset, uint32_t size) const;

        ///
        /// Creates a new buffer of provided specifications
        void Reserve(const void* data, uint32_t byte_size, bool static_buffer) const;

        void Bind() const;
        static void Unbind();

    private:
        unsigned int id_ = 0;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
