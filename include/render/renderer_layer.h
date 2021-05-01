// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDERER_LAYER_H
#define JACTORIO_INCLUDE_RENDER_RENDERER_LAYER_H
#pragma once

#include <glm/gtx/rotate_vector.hpp>
#include <memory>

#include "jactorio.h"

#include "core/coordinate_tuple.h"
#include "core/data_type.h"

namespace jactorio::render
{
    class IndexBuffer;
    class VertexArray;
    class VertexBuffer;

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
        using VertexPositionT = QuadPosition<Position2<float>>;

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

        /// Appends element to layer, after the highest element index where values were assigned
        /// \remark Ensure GlWriteBegin() has been called first before attempting to write into buffers
        FORCEINLINE void PushBack(const Element& element, VertexPositionT::PositionT::ValueT z) noexcept;
        FORCEINLINE void PushBack(const Element& element,
                                  VertexPositionT::PositionT::ValueT z,
                                  float rotate_deg) noexcept;

        /// Returns current element capacity of buffers
        J_NODISCARD uint32_t GetCapacity() const noexcept;

        /// Returns count of elements' index in buffers
        J_NODISCARD uint64_t GetIndicesCount() const noexcept;

        /// Queues allocation to hold element count
        /// \remark Performs reserves upon calling GlHandleBufferResize
        void Reserve(uint32_t count) noexcept;

        /// Queues resizing the buffer down to the initial size on upon construction
        /// \remark Performs reserves upon calling GlHandleBufferResize
        void ResizeDefault() noexcept;

        /// Signal to begin overriding old existing data in buffers
        /// \remark Does not guarantee that underlying buffers will be zeroed, merely that
        /// the data will not by uploaded with glBufferSubData
        void Clear() noexcept;

        // ======================================================================

        /// Begins writing data to buffers
        void GlWriteBegin() noexcept;
        /// Ends writing data to buffers
        void GlWriteEnd() noexcept;


        /// If a buffer resize was requested, resizes the buffers
        void GlHandleBufferResize();

        /// Deletes vertex and index buffers
        void GlDeleteBuffers() noexcept;

        /// Binds the vertex buffers, call this prior to drawing
        void GlBindBuffers() const noexcept;

    private:
        /// Handles detection of buffer resizing
        /// \return true if ok to push back into buffers, false if not
        FORCEINLINE bool PrePushBackChecks() noexcept;

        FORCEINLINE void SetBufferVertex(uint32_t buffer_index,
                                         const VertexPositionT& v_pos,
                                         VertexPositionT::PositionT::ValueT z) const noexcept;
        FORCEINLINE void SetBufferVertex(uint32_t buffer_index,
                                         const VertexPositionT& v_pos,
                                         VertexPositionT::PositionT::ValueT z,
                                         float rotate_deg) const noexcept;

        FORCEINLINE void SetBufferUv(uint32_t buffer_index, const UvPositionT& u_pos) const noexcept;

        /// Generates indices to draw tiles using the grid from gen_render_grid
        /// \returns Indices to be feed into Index_buffer
        static std::unique_ptr<unsigned int[]> GenRenderGridIndices(uint32_t tile_count);


        /// Initializes vertex buffers for rendering vertex and uv buffers + index buffer
        /// \remark Should only be called once
        void GlInitBuffers();

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

    inline void RendererLayer::PushBack(const Element& element, const VertexPositionT::PositionT::ValueT z) noexcept {
        if (!PrePushBackChecks())
            return;

        const auto buffer_index = nextElementIndex_ * kVbIndicesPerElement;
        nextElementIndex_++;

        SetBufferVertex(buffer_index, element.vertex, z);
        SetBufferUv(buffer_index, element.uv);
    }
    inline void RendererLayer::PushBack(const Element& element,
                                        const VertexPositionT::PositionT::ValueT z,
                                        const float rotate_deg) noexcept {
        if (!PrePushBackChecks())
            return;

        const auto buffer_index = nextElementIndex_ * kVbIndicesPerElement;
        nextElementIndex_++;

        SetBufferVertex(buffer_index, element.vertex, z, rotate_deg); // <-- Handles rotations
        SetBufferUv(buffer_index, element.uv);
    }

    inline bool RendererLayer::PrePushBackChecks() noexcept {
        assert(writeEnabled_);

        // Skips data appending operation if buffer needs to resize
        if (gResizeVertexBuffers_) {
            ++queuedECapacity_; // Will reserve an additional element for this one that was skipped
            return false;
        }

        if (nextElementIndex_ >= eCapacity_) {
            // No more space, reserve, will be repopulated on next update
            Reserve(eCapacity_ + 1);
            return false;
        }

        return true;
    }

    inline void RendererLayer::SetBufferVertex(const uint32_t buffer_index,
                                               const VertexPositionT& v_pos,
                                               VertexPositionT::PositionT::ValueT z) const noexcept {
        // Populate in following order: topL, topR, bottomR, bottomL (X Y)
        vertexBuffer_[buffer_index + 0] = v_pos.topLeft.x;
        vertexBuffer_[buffer_index + 1] = v_pos.topLeft.y;
        vertexBuffer_[buffer_index + 2] = z;

        vertexBuffer_[buffer_index + 3] = v_pos.bottomRight.x;
        vertexBuffer_[buffer_index + 4] = v_pos.topLeft.y;
        vertexBuffer_[buffer_index + 5] = z;

        vertexBuffer_[buffer_index + 6] = v_pos.bottomRight.x;
        vertexBuffer_[buffer_index + 7] = v_pos.bottomRight.y;
        vertexBuffer_[buffer_index + 8] = z;

        vertexBuffer_[buffer_index + 9]  = v_pos.topLeft.x;
        vertexBuffer_[buffer_index + 10] = v_pos.bottomRight.y;
        vertexBuffer_[buffer_index + 11] = z;
    }
    inline void RendererLayer::SetBufferVertex(const uint32_t buffer_index,
                                               const VertexPositionT& v_pos,
                                               const VertexPositionT::PositionT::ValueT z,
                                               const float rotate_deg) const noexcept {
        // Center origin (0, 0) on sprite
        const auto x_offset = (v_pos.bottomRight.x - v_pos.topLeft.x) / 2;
        assert(x_offset > 0);

        const auto y_offset = (v_pos.bottomRight.y - v_pos.topLeft.y) / 2;
        assert(y_offset > 0);

        glm::vec2 tl(-x_offset, -y_offset);
        glm::vec2 tr(x_offset, -y_offset);
        glm::vec2 bl(-x_offset, y_offset);
        glm::vec2 br(x_offset, y_offset);

        // Rotate
        // Since they are rotated, they can't be drawn using only top left and bottom right
        const float rotate_rad = glm::radians(rotate_deg);

        tl = rotate(tl, rotate_rad);
        tr = rotate(tr, rotate_rad);
        bl = rotate(bl, rotate_rad);
        br = rotate(br, rotate_rad);

        // Move to position
        tl.x += x_offset + v_pos.topLeft.x;
        tl.y += y_offset + v_pos.topLeft.y;

        tr.x += v_pos.bottomRight.x - x_offset;
        tr.y += y_offset + v_pos.topLeft.y;

        bl.x += x_offset + v_pos.topLeft.x;
        bl.y += v_pos.bottomRight.y - y_offset;

        br.x += v_pos.bottomRight.x - x_offset;
        br.y += v_pos.bottomRight.y - y_offset;

        // Populate in following order: topL, topR, bottomR, bottomL (X Y)

        vertexBuffer_[buffer_index + 0] = tl.x;
        vertexBuffer_[buffer_index + 1] = tl.y;
        vertexBuffer_[buffer_index + 2] = z;

        vertexBuffer_[buffer_index + 3] = tr.x;
        vertexBuffer_[buffer_index + 4] = tr.y;
        vertexBuffer_[buffer_index + 5] = z;

        vertexBuffer_[buffer_index + 6] = br.x;
        vertexBuffer_[buffer_index + 7] = br.y;
        vertexBuffer_[buffer_index + 8] = z;

        vertexBuffer_[buffer_index + 9]  = bl.x;
        vertexBuffer_[buffer_index + 10] = bl.y;
        vertexBuffer_[buffer_index + 11] = z;
    }

    inline void RendererLayer::SetBufferUv(const uint32_t buffer_index, const UvPositionT& u_pos) const noexcept {
        // Populate in following order: bottomL, bottomR, topR, topL (X Y) (NOT THE SAME AS ABOVE!!)

        // UV
        uvBuffer_[buffer_index + 0] = u_pos.topLeft.x;
        uvBuffer_[buffer_index + 1] = u_pos.bottomRight.y;

        uvBuffer_[buffer_index + 3] = u_pos.bottomRight.x;
        uvBuffer_[buffer_index + 4] = u_pos.bottomRight.y;

        uvBuffer_[buffer_index + 6] = u_pos.bottomRight.x;
        uvBuffer_[buffer_index + 7] = u_pos.topLeft.y;

        uvBuffer_[buffer_index + 9]  = u_pos.topLeft.x;
        uvBuffer_[buffer_index + 10] = u_pos.topLeft.y;
    }

} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDERER_LAYER_H
