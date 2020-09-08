// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_INDEX_BUFFER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_INDEX_BUFFER_H
#pragma once

#include "jactorio.h"

namespace jactorio::renderer
{
    class IndexBuffer
    {
    public:
        ///
        /// \param count Number of indices
        IndexBuffer(const unsigned int* data, unsigned int count);
        ~IndexBuffer();

        IndexBuffer(const IndexBuffer& other)     = delete;
        IndexBuffer(IndexBuffer&& other) noexcept = delete;
        IndexBuffer& operator=(const IndexBuffer& other) = delete;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept = delete;

        ///
        /// Creates a new buffer of provided specifications
        void Reserve(const void* data, uint32_t index_count);

        void Bind() const;
        static void Unbind();

        J_NODISCARD unsigned int Count() const;

    private:
        unsigned int id_    = 0;
        unsigned int count_ = 0;
    };
} // namespace jactorio::renderer

#endif // JACTORIO_INCLUDE_RENDERER_OPENGL_INDEX_BUFFER_H
