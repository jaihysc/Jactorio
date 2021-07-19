// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/irender_buffer.h"

using namespace jactorio;

void render::IRenderBuffer::GlInit() noexcept {
    vtxBuffer_.Init();
    idxBuffer_.Init();

    ResizeDefault();
    GlHandleBufferResize();
}

uint32_t render::IRenderBuffer::VtxCapacity() const noexcept {
    return vtxCapacity_;
}

uint32_t render::IRenderBuffer::IdxCapacity() const noexcept {
    return idxCapacity_;
}

void render::IRenderBuffer::ReserveVtx(const uint32_t vtx_capacity) noexcept {
    assert(vtx_capacity > 0); // Count must be greater than 0
    queuedVtxCapacity_ = vtx_capacity;
    resizeBuffers_     = true;
}

void render::IRenderBuffer::ReserveIdx(const uint32_t idx_capacity) noexcept {
    assert(idx_capacity > 0); // Count must be greater than 0
    queuedIdxCapacity_ = idx_capacity;
    resizeBuffers_     = true;
}

void render::IRenderBuffer::ResizeDefault() noexcept {
    ReserveVtx(kInitialSize);
    ReserveIdx(kInitialSize);
}

void render::IRenderBuffer::GlWriteBegin() noexcept {
    assert(!writeEnabled_);
    assert(vtxCapacity_ > 0); // Mapping fails unless capacity is at least 1
    assert(idxCapacity_ > 0);

    vtxBase_ = static_cast<ImDrawVert*>(vtxBuffer_.Map());
    idxBase_ = static_cast<ImDrawIdx*>(idxBuffer_.Map());

    vtxWrite_ = vtxBase_;
    idxWrite_ = idxBase_;

    writeEnabled_ = true;
}

void render::IRenderBuffer::GlWriteEnd() noexcept {
    assert(writeEnabled_);

    vtxBuffer_.UnMap();
    idxBuffer_.UnMap();

    writeEnabled_ = false;
}

void render::IRenderBuffer::GlHandleBufferResize() noexcept {
    if (!resizeBuffers_) {
        return;
    }

    resizeBuffers_ = false;
    vtxCapacity_   = LossyCast<uint32_t>(SafeCast<float>(queuedVtxCapacity_) * kResizeCapacityMultiplier);
    idxCapacity_   = LossyCast<uint32_t>(SafeCast<float>(queuedIdxCapacity_) * kResizeCapacityMultiplier);

    vtxBuffer_.Reserve(nullptr, vtxCapacity_ * sizeof(ImDrawVert), false);
    idxBuffer_.Reserve(nullptr, idxCapacity_ * sizeof(ImDrawIdx));

    LOG_MESSAGE_F(debug, "Imgui buffer resized to %u %u", vtxCapacity_, idxCapacity_);
}

void render::IRenderBuffer::GlBind() const noexcept {
    vtxBuffer_.Bind();
    idxBuffer_.Bind();
}
