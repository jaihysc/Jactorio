// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/trender_buffer.h"

#include "core/convert.h"

using namespace jactorio;

render::TRenderBuffer::TRenderBuffer() {
    baseVb_.Init();
    vertexArray_.Init();
    vertexArray_.AddBuffer(&baseVb_, kBaseValsPerElement, 0);

    Reserve(kInitialSize);
    GlHandleBufferResize();
}


uint32_t render::TRenderBuffer::Capacity() const noexcept {
    return eCapacity_;
}

void render::TRenderBuffer::Reserve(const uint32_t count) noexcept {
    assert(count > 0); // Count must be greater than 0

    gResizeVertexBuffers_ = true;
    queuedECapacity_      = count;
    LOG_MESSAGE(debug, "Queuing buffer resize");
}

void render::TRenderBuffer::ResizeDefault() noexcept {
    Reserve(kInitialSize);
}

void render::TRenderBuffer::Clear() noexcept {
    writePtr_ = baseBuffer_;
}

void render::TRenderBuffer::GlWriteBegin() noexcept {
    assert(!writeEnabled_);
    assert(eCapacity_ > 0); // Mapping fails unless capacity is at least 1

    baseBuffer_ = static_cast<VertexArray::ElementT*>(baseVb_.Map());
    writePtr_   = baseBuffer_;

    writeEnabled_ = true;
}

void render::TRenderBuffer::GlWriteEnd() noexcept {
    assert(writeEnabled_);

    baseVb_.UnMap();

    writeEnabled_ = false;
}

void render::TRenderBuffer::GlHandleBufferResize() {
    // Only resize if resize is set
    if (!gResizeVertexBuffers_)
        return;

    gResizeVertexBuffers_ = false;
    eCapacity_ = LossyCast<decltype(eCapacity_)>(SafeCast<float>(queuedECapacity_) * kResizeECapacityMultiplier);

    baseVb_.Reserve(nullptr, eCapacity_ * kBaseBytesPerElement, false);

    LOG_MESSAGE_F(debug, "Buffer resized to %d", eCapacity_);
}

void render::TRenderBuffer::GlBindBuffers() const noexcept {
    vertexArray_.Bind();
    baseVb_.Bind();
}
