// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_ARRAY_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_ARRAY_H
#pragma once

namespace jactorio::render
{
    class VertexBuffer;

    class VertexArray
    {
        unsigned int id_ = 0;

    public:
        VertexArray();
        ~VertexArray();

        VertexArray(const VertexArray& other)     = delete;
        VertexArray(VertexArray&& other) noexcept = delete;
        VertexArray& operator=(const VertexArray& other) = delete;
        VertexArray& operator=(VertexArray&& other) noexcept = delete;

        /// Adds specified buffer to the vertex array
        /// \remark Vertex_buffer must be deleted manually, it is not managed by the vertex array
        /// \param vb Vertex buffer to add to vertex array
        /// \param span Number of floats for one set of coordinates (2 for X and Y)
        /// \param location Slot in vertex array in which vertex buffer is placed
        /// This must be managed manually to avoid conflicts
        void AddBuffer(const VertexBuffer* vb, unsigned span, unsigned location) const;

        void Bind() const;
        static void Unbind();
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_VERTEX_ARRAY_H
