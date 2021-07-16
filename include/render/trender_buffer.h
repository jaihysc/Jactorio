// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_TRENDER_BUFFER_H
#define JACTORIO_INCLUDE_RENDER_TRENDER_BUFFER_H
#pragma once

#include "jactorio.h"

#include "core/convert.h"
#include "core/coordinate_tuple.h"
#include "core/data_type.h"
#include "render/opengl/vertex_array.h"
#include "render/opengl/vertex_buffer.h"

namespace jactorio::render
{
    /// Generates and maintains the buffers for tile renderer
    /// \remark Can only be created and destructed in an Opengl Context
    /// \remark Methods with Gl prefix must be called from an OpenGl context
    class TRenderBuffer
    {
        /// How many elements to reserve upon construction
        static constexpr uint32_t kInitialSize = 500;

        /// When resizing, the amount requested is multiplied by this, reducing future allocations
        static constexpr double kResizeECapacityMultiplier = 1.25;

        // Vertex buffer
        // It is actually 5ms faster when each tile is 4 components! Just put 0 for Z if depth testing not needed
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

        TRenderBuffer();

        // Copying is disallowed because this needs to interact with openGL

        TRenderBuffer(const TRenderBuffer& other)     = delete;
        TRenderBuffer(TRenderBuffer&& other) noexcept = default;
        TRenderBuffer& operator=(const TRenderBuffer& other) = delete;
        TRenderBuffer& operator=(TRenderBuffer&& other) noexcept = default;


        /// Appends element
        /// \remark Ensure GlWriteBegin() has been called first before calling this
        FORCEINLINE void PushBack(const Element& element) noexcept;

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

    inline void TRenderBuffer::PushBack(const Element& element) noexcept {
        if (!PrePushBackChecks())
            return;

        UncheckedPushBack(element);
    }

    inline void TRenderBuffer::UncheckedPushBack(const Element& element) noexcept {
        writePtr_[0] = element.vertex.x;
        writePtr_[1] = element.vertex.y;
        writePtr_[2] = element.vertex.z;
        writePtr_[3] = element.texCoordIndex;
        writePtr_ += 4;
    }

    inline bool TRenderBuffer::PrePushBackChecks() noexcept {
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

    inline uint32_t TRenderBuffer::Size() const noexcept {
        return SafeCast<uint32_t>((writePtr_ - baseBuffer_) / kBaseValsPerElement);
    }
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_TRENDER_BUFFER_H
