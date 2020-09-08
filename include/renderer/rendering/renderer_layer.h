// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
#pragma once

#include <memory>

#include "jactorio.h"

#include "core/coordinate_tuple.h"
#include "core/data_type.h"
#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/opengl/vertex_buffer.h"

namespace jactorio::renderer
{
    ///
    /// Generates and maintains the buffers of a rendering layer
    /// Currently 2 buffers are used: Vertex and UV
    /// \remark Can only be created and destructed in an Opengl Context
    /// \remark Methods with Gl prefix must be called from an OpenGl context
    class RendererLayer
    {
        /// How many elements to reserve upon construction
        static constexpr uint32_t kInitialSize = 500;

        /// When resizing, the amount requested is multiplied by this, reducing future allocations
        static constexpr double kResizeECapacityMultiplier = 1.25;

        /// Vertex buffer
        static constexpr int kVbIndicesPerCoordinate = 3;

        static constexpr int kVbIndicesPerElement = kVbIndicesPerCoordinate * 4;
        static constexpr int kVbBytesPerElement   = kVbIndicesPerElement * sizeof(float);

        /// Uv buffer
        static constexpr int kUvIndicesPerCoordinate = 3;

        /// Index buffer
        static constexpr int kInIndicesPerElement = 6;


        // Opengl seems to require the same spans
        static_assert(kVbIndicesPerCoordinate == kUvIndicesPerCoordinate);

    public:
        using VertexPositionT = core::QuadPosition<core::Position2<float>>;

        struct Element
        {
            Element() = default;

            Element(const VertexPositionT& vertex, const UvPositionT& uv) : vertex(vertex), uv(uv) {}

            VertexPositionT vertex;
            UvPositionT uv;
        };

        explicit RendererLayer();
        ~RendererLayer();

        // Copying is disallowed because this needs to interact with openGL

        RendererLayer(const RendererLayer& other)     = delete;
        RendererLayer(RendererLayer&& other) noexcept = default;
        RendererLayer& operator=(const RendererLayer& other) = delete;
        RendererLayer& operator=(RendererLayer&& other) noexcept = default;

        // ======================================================================

        ///
        /// Appends element to layer, after the highest element index where values were assigned
        /// \remark Ensure GlWriteBegin() has been called first before attempting to write into buffers
        void PushBack(const Element& element, VertexPositionT::PositionT::ValueT z) noexcept;
        void PushBack(const Element& element, VertexPositionT::PositionT::ValueT z, float rotate_deg) noexcept;

        ///
        /// Returns current element capacity of buffers
        J_NODISCARD uint32_t GetCapacity() const noexcept;

        ///
        /// Returns count of elements' index in buffers
        J_NODISCARD uint64_t GetIndicesCount() const noexcept;

        ///
        /// Queues allocation to hold element count
        /// \remark Performs reserves upon calling GlHandleBufferResize
        void Reserve(uint32_t count) noexcept;

        ///
        /// Queues resizing the buffer down to the initial size on upon construction
        /// \remark Performs reserves upon calling GlHandleBufferResize
        void ResizeDefault() noexcept;

        ///
        /// Signal to begin overriding old existing data in buffers
        /// \remark Does not guarantee that underlying buffers will be zeroed, merely that
        /// the data will not by uploaded with glBufferSubData
        void Clear() noexcept;

        // ======================================================================

        ///
        /// Begins writing data to buffers
        void GlWriteBegin() noexcept;
        ///
        /// Ends writing data to buffers
        void GlWriteEnd() noexcept;


        ///
        /// If a buffer resize was requested, resizes the buffers
        void GlHandleBufferResize();

        ///
        /// Deletes vertex and index buffers
        void GlDeleteBuffers() noexcept;

        ///
        /// Binds the vertex buffers, call this prior to drawing
        void GlBindBuffers() const noexcept;

    private:
        ///
        /// Handles detection of buffer resizing
        /// \return true if ok to push back into buffers, false if not
        bool PrePushBackChecks() noexcept;

        void SetBufferVertex(uint32_t buffer_index,
                             const VertexPositionT& v_pos,
                             VertexPositionT::PositionT::ValueT z) const noexcept;
        void SetBufferVertex(uint32_t buffer_index,
                             const VertexPositionT& v_pos,
                             VertexPositionT::PositionT::ValueT z,
                             float rotate_deg) const noexcept;

        void SetBufferUv(uint32_t buffer_index, const UvPositionT& u_pos) const noexcept;

        ///
        /// Generates indices to draw tiles using the grid from gen_render_grid
        /// \returns Indices to be feed into Index_buffer
        static std::unique_ptr<unsigned int[]> GenRenderGridIndices(uint32_t tile_count);


        ///
        /// Initializes vertex buffers for rendering vertex and uv buffers + index buffer
        /// \remark Should only be called once
        void GlInitBuffers();

        ///
        /// Only deletes heap vertex buffers, does not set pointers to nullptr
        void GlFreeBuffers() const noexcept;

        // ======================================================================


        /// Buffer index to insert the next element on push_back
        uint32_t nextElementIndex_ = 0;

        /// Element capacity which will be requested on the next GUpdateData
        uint32_t queuedECapacity_ = 0;

        /// Current *element* capacity of VERTEX buffers
        /// for index buffer size, multiply by 6,
        uint32_t eCapacity_ = 0;

        bool writeEnabled_         = false;
        bool gResizeVertexBuffers_ = false;

        float* vertexBuffer_ = nullptr;
        float* uvBuffer_     = nullptr;

        VertexArray* vertexArray_ = nullptr;
        VertexBuffer* vertexVb_   = nullptr;
        VertexBuffer* uvVb_       = nullptr;
        IndexBuffer* indexIb_     = nullptr;
    };
} // namespace jactorio::renderer

#endif // JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
