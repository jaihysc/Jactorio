// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_IRENDER_BUFFER_H
#define JACTORIO_INCLUDE_RENDER_IRENDER_BUFFER_H
#pragma once

#include <imgui.h>

#include "jactorio.h"

#include "core/convert.h"
#include "render/opengl/index_buffer.h"
#include "render/opengl/vertex_buffer.h"

namespace jactorio::render
{
    /// Generates and maintains the buffers for imgui renderer
    /// \remark Can only be Init() and destructed in an Opengl Context
    /// \remark Methods with Gl prefix must be called from an OpenGl context
    class IRenderBuffer
    {
        /// How many elements to reserve upon construction
        static constexpr uint32_t kInitialSize = 500;

        /// When resizing, gain at least this amount
        static constexpr auto kResizeGain = 500;

        /// When resizing, the amount requested is multiplied by this
        static constexpr auto kResizeCapacityMultiplier = 1.25f;

    public:
        IRenderBuffer() = default;

        // Copying is disallowed because this needs to interact with openGL

        IRenderBuffer(const IRenderBuffer& other)     = delete;
        IRenderBuffer(IRenderBuffer&& other) noexcept = default;
        IRenderBuffer& operator=(const IRenderBuffer& other) = delete;
        IRenderBuffer& operator=(IRenderBuffer&& other) noexcept = default;

        /// Init the buffers
        void GlInit() noexcept;

        /// Adds vert to buffer
        FORCEINLINE void PushVtx(const ImDrawVert& vtx) noexcept;
        /// Adds index to buffer
        FORCEINLINE void PushIdx(ImDrawIdx idx) noexcept;

        /// Adds vert to buffer without checking for sufficient capacity
        FORCEINLINE void UncheckedPushVtx(const ImDrawVert& vtx) noexcept;
        /// Adds index to buffer without checking for sufficient capacity
        FORCEINLINE void UncheckedPushIdx(ImDrawIdx idx) noexcept;

        /// \return Number of vertices in vertex buffer
        J_NODISCARD FORCEINLINE uint32_t VtxCount() const noexcept;
        /// \return Number of indices in index buffer
        J_NODISCARD FORCEINLINE uint32_t IdxCount() const noexcept;

        /// \return Max number of vertices in vertex buffer
        J_NODISCARD uint32_t VtxCapacity() const noexcept;
        /// \return Max number of indices in index buffer
        J_NODISCARD uint32_t IdxCapacity() const noexcept;

        /// Queues allocation to have specified capacity in vertex buffer
        /// \remark Performs reserves upon calling GlHandleBufferResize
        void ReserveVtx(uint32_t vtx_capacity) noexcept;
        /// Queues allocation to have specified capacity in index buffer
        /// \remark Performs reserves upon calling GlHandleBufferResize
        void ReserveIdx(uint32_t idx_capacity) noexcept;

        /// Queues resizing to the initial size upon construction
        /// \remark Performs reserves upon calling GlHandleBufferResize
        void ResizeDefault() noexcept;

        /// Begins writing data to buffers
        void GlWriteBegin() noexcept;
        /// Ends writing data to buffers
        void GlWriteEnd() noexcept;

        /// If a buffer resize was requested, resizes the buffers
        void GlHandleBufferResize() noexcept;

        /// Binds the vertex buffers, call this prior to drawing
        void GlBind() const noexcept;

    private:
        VertexBuffer vtxBuffer_;
        IndexBuffer idxBuffer_;

        // Number of elements(ImDrawVert, ImDrawIdx) which can be held
        uint32_t vtxCapacity_ = 0;
        uint32_t idxCapacity_ = 0;

        // Capacity requested for next GlHandleBufferResize()
        uint32_t queuedVtxCapacity_ = 0;
        uint32_t queuedIdxCapacity_ = 0;

        // Location of mapped buffer
        ImDrawVert* vtxBase_ = nullptr;
        ImDrawIdx* idxBase_  = nullptr;

        // Location to write next into mapped buffer
        ImDrawVert* vtxWrite_ = nullptr;
        ImDrawIdx* idxWrite_  = nullptr;

        bool writeEnabled_  = false; // Debug check to prevent writing when unmapped
        bool resizeBuffers_ = false;
    };

    inline void IRenderBuffer::PushVtx(const ImDrawVert& vtx) noexcept {
        assert(writeEnabled_);

        if (VtxCount() >= vtxCapacity_) {
            queuedVtxCapacity_ = vtxCapacity_ + kResizeGain;
            resizeBuffers_     = true;
            return;
        }

        UncheckedPushVtx(vtx);
    }

    inline void IRenderBuffer::PushIdx(const ImDrawIdx idx) noexcept {
        assert(writeEnabled_);

        if (IdxCount() >= idxCapacity_) {
            queuedIdxCapacity_ = idxCapacity_ + kResizeGain;
            resizeBuffers_     = true;
            return;
        }

        UncheckedPushIdx(idx);
    }

    inline void IRenderBuffer::UncheckedPushVtx(const ImDrawVert& vtx) noexcept {
        vtxWrite_[0] = vtx;
        ++vtxWrite_;
    }

    inline void IRenderBuffer::UncheckedPushIdx(const ImDrawIdx idx) noexcept {
        idxWrite_[0] = idx;
        ++idxWrite_;
    }

    inline uint32_t IRenderBuffer::VtxCount() const noexcept {
        return SafeCast<uint32_t>(vtxWrite_ - vtxBase_);
    }

    inline uint32_t IRenderBuffer::IdxCount() const noexcept {
        return SafeCast<uint32_t>(idxWrite_ - idxBase_);
    }

} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_IRENDER_BUFFER_H
