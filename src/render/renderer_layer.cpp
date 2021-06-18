// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/renderer_layer.h"

#include "core/convert.h"

using namespace jactorio;

render::RendererLayer::RendererLayer() : baseVb_(baseBuffer_, eCapacity_ * kBaseBytesPerElement, false) {
    vertexArray_.AddBuffer(&baseVb_, kBaseValsPerElement, 0);

    Reserve(kInitialSize);
    GlHandleBufferResize();
}


uint32_t render::RendererLayer::GetCapacity() const noexcept {
    return eCapacity_;
}

uint32_t render::RendererLayer::GetElementCount() const noexcept {
    return nextElementIndex_;
}

void render::RendererLayer::Reserve(const uint32_t count) noexcept {
    assert(count > 0); // Count must be greater than 0

    gResizeVertexBuffers_ = true;
    queuedECapacity_      = count;
    LOG_MESSAGE(debug, "Queuing buffer resize");
}

void render::RendererLayer::ResizeDefault() noexcept {
    Reserve(kInitialSize);
}

void render::RendererLayer::Clear() noexcept {
    nextElementIndex_ = 0;
}

void render::RendererLayer::GlWriteBegin() noexcept {
    assert(!writeEnabled_);
    assert(eCapacity_ > 0); // Mapping fails unless capacity is at least 1

    baseBuffer_ = static_cast<VertexArray::ElementT*>(baseVb_.Map());

    writeEnabled_ = true;
}

void render::RendererLayer::GlWriteEnd() noexcept {
    assert(writeEnabled_);

    baseVb_.UnMap();

    writeEnabled_ = false;
}

void render::RendererLayer::GlHandleBufferResize() {
    // Only resize if resize is set
    if (!gResizeVertexBuffers_)
        return;

    gResizeVertexBuffers_ = false;
    eCapacity_            = LossyCast<decltype(eCapacity_)>(queuedECapacity_ * kResizeECapacityMultiplier);

    baseVb_.Reserve(nullptr, eCapacity_ * kBaseBytesPerElement, false);

    LOG_MESSAGE_F(debug, "Buffer resized to %d", eCapacity_);
}

void render::RendererLayer::GlBindBuffers() const noexcept {
    vertexArray_.Bind();
    baseVb_.Bind();
}
