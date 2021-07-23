// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_ARRAY_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_ARRAY_H
#pragma once

#include <utility>

namespace jactorio::render
{
    class VertexBuffer;

    /// \remark Lifetime of object must be in opengl context
    class VertexArray
    {
    public:
        using ElementT = unsigned short;

        VertexArray() = default;
        ~VertexArray();

        VertexArray(const VertexArray& other) = delete;
        VertexArray(VertexArray&& other) noexcept;

        VertexArray& operator=(VertexArray other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(VertexArray& lhs, VertexArray& rhs) noexcept {
            using std::swap;
            swap(lhs.id_, rhs.id_);
        }

        /// Generates buffer
        void Init() noexcept;

        /// Adds specified buffer to the vertex array
        /// \remark vb must be deleted manually, it is not managed by the vertex array
        /// \param vb Vertex buffer to add to vertex array
        /// \param span Number of floats for one set of coordinates (2 for X and Y)
        /// \param location Slot in vertex array in which vertex buffer is placed
        /// This must be managed manually to avoid conflicts
        void AddBuffer(const VertexBuffer* vb, unsigned span, unsigned location) const noexcept;

        void Bind() const noexcept;
        static void Unbind() noexcept;

    private:
        unsigned int id_ = 0;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_ARRAY_H
