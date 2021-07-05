// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDERER_LAYER_H
#define JACTORIO_INCLUDE_RENDER_RENDERER_LAYER_H
#pragma once

#include <glm/gtx/rotate_vector.hpp>

#include "jactorio.h"

#include "core/convert.h"
#include "core/coordinate_tuple.h"
#include "core/data_type.h"
#include "render/opengl/vertex_array.h"
#include "render/opengl/vertex_buffer.h"

namespace jactorio::render
{
    /// Generates and maintains the buffers of a rendering layer
    /// \remark Can only be created and destructed in an Opengl Context
    /// \remark Methods with Gl prefix must be called from an OpenGl context
    class RendererLayer
    {
        /// How many elements to reserve upon construction
        static constexpr uint32_t kInitialSize = 500;

        /// When resizing, the amount requested is multiplied by this, reducing future allocations
        static constexpr double kResizeECapacityMultiplier = 1.25;

        // Vertex buffer
        static constexpr int kBaseValsPerElement  = 4;
        static constexpr int kBaseBytesPerElement = kBaseValsPerElement * sizeof(VertexArray::ElementT);

    public:
        using VertexPositionT = Position3<VertexArray::ElementT>;

        struct Element
        {
            Element(const VertexPositionT vertex, const SpriteTexCoordIndexT tex_coord_index)
                : vertex(vertex), texCoordIndex(tex_coord_index) {}

            VertexPositionT vertex;
            SpriteTexCoordIndexT texCoordIndex;

            // VertexArray expects 4 elements of same size
            static_assert(sizeof(texCoordIndex) == sizeof(VertexArray::ElementT));
        };

        explicit RendererLayer();

        // Copying is disallowed because this needs to interact with openGL

        RendererLayer(const RendererLayer& other)     = delete;
        RendererLayer(RendererLayer&& other) noexcept = default;
        RendererLayer& operator=(const RendererLayer& other) = delete;
        RendererLayer& operator=(RendererLayer&& other) noexcept = default;


        /// Appends element
        /// \remark Ensure GlWriteBegin() has been called first before calling this
        FORCEINLINE void PushBack(const Element& element) noexcept;

        /// Appends element
        /// \remark Ensure GlWriteBegin() has been called first before calling this
        FORCEINLINE void PushBack(const Element& element, float rotate_deg) noexcept;

        /// Appends element without checking for sufficient capacity
        /// \remark Ensure GlWriteBegin() has been called first before calling this
        FORCEINLINE void UncheckedPushBack(const Element& element) noexcept;


        /// Returns current element capacity of buffers
        J_NODISCARD uint32_t Capacity() const noexcept;

        /// Returns count of elements in buffers
        J_NODISCARD FORCEINLINE uint32_t Size() const noexcept;

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


        /// Begins writing data to buffers
        void GlWriteBegin() noexcept;
        /// Ends writing data to buffers
        void GlWriteEnd() noexcept;


        /// If a buffer resize was requested, resizes the buffers
        void GlHandleBufferResize();

        /// Binds the vertex buffers, call this prior to drawing
        void GlBindBuffers() const noexcept;

    private:
        /// Handles detection of buffer resizing
        /// \return true if ok to push back into buffers, false if not
        FORCEINLINE bool PrePushBackChecks() noexcept;

        // TODO reimplement rotation
        FORCEINLINE void SetBufferVertex(const Element& element, float rotate_deg) const noexcept;


        /// Element capacity which will be requested on the next GUpdateData
        uint32_t queuedECapacity_ = 0;

        /// Current *element* capacity of VERTEX buffers
        /// for index buffer size, multiply by 6,
        uint32_t eCapacity_ = 0;

        bool writeEnabled_         = false;
        bool gResizeVertexBuffers_ = false;

        VertexArray::ElementT* baseBuffer_ = nullptr;

        /// Location to insert next element in baseBuffer_ on PushBack
        VertexArray::ElementT* writePtr_ = nullptr;

        VertexArray vertexArray_;
        VertexBuffer baseVb_;
    };

    inline void RendererLayer::PushBack(const Element& element) noexcept {
        if (!PrePushBackChecks())
            return;

        UncheckedPushBack(element);
    }

    inline void RendererLayer::PushBack(const Element& element, const float rotate_deg) noexcept {
        if (!PrePushBackChecks())
            return;

        SetBufferVertex(element, rotate_deg);
    }

    inline void RendererLayer::UncheckedPushBack(const Element& element) noexcept {
        writePtr_[0] = element.vertex.x;
        writePtr_[1] = element.vertex.y;
        writePtr_[2] = element.vertex.z;
        writePtr_[3] = element.texCoordIndex;
        writePtr_ += 4;
    }

    inline bool RendererLayer::PrePushBackChecks() noexcept {
        assert(writeEnabled_);

        // Skips data appending operation if buffer needs to resize
        if (gResizeVertexBuffers_) {
            ++queuedECapacity_; // Will reserve an additional element for this one that was skipped
            return false;
        }

        if (Size() >= eCapacity_) {
            // No more space, reserve, will be repopulated on next update
            Reserve(eCapacity_ + 1);
            return false;
        }

        return true;
    }


    inline void RendererLayer::SetBufferVertex(const Element& element, const float rotate_deg) const noexcept {
        /* // TODO
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
        */
    }

    inline uint32_t RendererLayer::Size() const noexcept {
        return SafeCast<uint32_t>((writePtr_ - baseBuffer_) / kBaseValsPerElement);
    }
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDERER_LAYER_H
