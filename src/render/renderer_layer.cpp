// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/renderer_layer.h"
#include "render/opengl/vertex_array.h"

#include "core/convert.h"

#include "game/player/player.h"

#include "render/opengl/index_buffer.h"
#include "render/opengl/vertex_array.h"
#include "render/opengl/vertex_buffer.h"

using namespace jactorio;

render::RendererLayer::RendererLayer() {
    GlInitBuffers();
    Reserve(kInitialSize);
    GlHandleBufferResize();
}

render::RendererLayer::~RendererLayer() {
    GlFreeBuffers();
}

// ======================================================================

uint32_t render::RendererLayer::GetCapacity() const noexcept {
    return eCapacity_;
}

uint64_t render::RendererLayer::GetIndicesCount() const noexcept {
    return SafeCast<uint64_t>(nextElementIndex_) * kInIndicesPerElement;
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

    vertexBuffer_ = static_cast<float*>(vertexVb_->Map());
    uvBuffer_     = static_cast<float*>(uvVb_->Map());

    writeEnabled_ = true;
}

void render::RendererLayer::GlWriteEnd() noexcept {
    assert(writeEnabled_);

    vertexVb_->UnMap();
    uvVb_->UnMap();

    writeEnabled_ = false;
}

void render::RendererLayer::GlHandleBufferResize() {
    // Only resize if resize is set
    if (!gResizeVertexBuffers_)
        return;

    gResizeVertexBuffers_ = false;
    eCapacity_            = LossyCast<decltype(eCapacity_)>(queuedECapacity_ * kResizeECapacityMultiplier);

    vertexVb_->Reserve(nullptr, eCapacity_ * kVbBytesPerElement, false);
    uvVb_->Reserve(nullptr, eCapacity_ * kVbBytesPerElement, false);

    // Index buffer
    const auto data = GenRenderGridIndices(eCapacity_);
    indexIb_->Reserve(data.get(), eCapacity_ * kInIndicesPerElement);

    LOG_MESSAGE_F(debug, "Buffer resized to %d", eCapacity_);
}

void render::RendererLayer::GlDeleteBuffers() noexcept {
    GlFreeBuffers();

    vertexArray_ = nullptr;
    vertexVb_    = nullptr;
    uvVb_        = nullptr;
    indexIb_     = nullptr;
}

void render::RendererLayer::GlBindBuffers() const noexcept {
    vertexArray_->Bind();
    vertexVb_->Bind();
    uvVb_->Bind();
    indexIb_->Bind();
}

// ======================================================================

std::unique_ptr<unsigned int[]> render::RendererLayer::GenRenderGridIndices(const uint32_t tile_count) {
    // Indices generation pattern:
    // top left
    // top right
    // bottom right
    // bottom left

    std::unique_ptr<unsigned int[]> positions(new unsigned int[SafeCast<uint64_t>(tile_count) * kInIndicesPerElement]);

    unsigned int positions_index    = 0;
    unsigned int index_buffer_index = 0; // Index to be saved into positions

    for (uint32_t i = 0; i < tile_count; ++i) {
        positions[positions_index++] = index_buffer_index;
        positions[positions_index++] = index_buffer_index + 1;
        positions[positions_index++] = index_buffer_index + 2;

        positions[positions_index++] = index_buffer_index + 2;
        positions[positions_index++] = index_buffer_index + 3;
        positions[positions_index++] = index_buffer_index;

        index_buffer_index += 4;
    }

    return positions;
}

void render::RendererLayer::GlInitBuffers() {
    assert(vertexVb_ == nullptr);
    assert(uvVb_ == nullptr);

    vertexVb_ = new VertexBuffer(vertexBuffer_, eCapacity_ * kVbBytesPerElement, false);
    uvVb_     = new VertexBuffer(uvBuffer_, eCapacity_ * kVbBytesPerElement, false);

    vertexArray_ = new VertexArray();
    // Register buffer properties and shader location
    // Vertex - location 0
    // UV - location 1
    vertexArray_->AddBuffer(vertexVb_, kVbIndicesPerCoordinate, 0);
    vertexArray_->AddBuffer(uvVb_, kUvIndicesPerCoordinate, 1);

    // Index buffer
    // As of right now there is no point holding onto the index_buffer data pointer after handing it to the GPu
    // since it is never modified
    const auto data = GenRenderGridIndices(eCapacity_);
    indexIb_        = new IndexBuffer(data.get(), eCapacity_ * kInIndicesPerElement);

    // No need to resize anymore
    gResizeVertexBuffers_ = false;
}

void render::RendererLayer::GlFreeBuffers() const noexcept {
    delete vertexArray_;
    delete vertexVb_;
    delete uvVb_;
    delete indexIb_;
}
