// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/rendering/renderer_layer.h"
#include "render/opengl/vertex_array.h"

#include <glm/gtx/rotate_vector.hpp>

#include "core/convert.h"

#include "game/player/player_data.h"

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

void render::RendererLayer::PushBack(const Element& element, const VertexPositionT::PositionT::ValueT z) noexcept {
    if (!PrePushBackChecks())
        return;

    const auto buffer_index = nextElementIndex_ * kVbIndicesPerElement;
    nextElementIndex_++;

    SetBufferVertex(buffer_index, element.vertex, z);
    SetBufferUv(buffer_index, element.uv);
}

void render::RendererLayer::PushBack(const Element& element,
                                     const VertexPositionT::PositionT::ValueT z,
                                     const float rotate_deg) noexcept {
    if (!PrePushBackChecks())
        return;

    const auto buffer_index = nextElementIndex_ * kVbIndicesPerElement;
    nextElementIndex_++;

    SetBufferVertex(buffer_index, element.vertex, z, rotate_deg); // <-- Handles rotations
    SetBufferUv(buffer_index, element.uv);
}

uint32_t render::RendererLayer::GetCapacity() const noexcept {
    return eCapacity_;
}

uint64_t render::RendererLayer::GetIndicesCount() const noexcept {
    return core::SafeCast<uint64_t>(nextElementIndex_) * kInIndicesPerElement;
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
    eCapacity_            = core::LossyCast<decltype(eCapacity_)>(queuedECapacity_ * kResizeECapacityMultiplier);

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

bool render::RendererLayer::PrePushBackChecks() noexcept {
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

void render::RendererLayer::SetBufferVertex(const uint32_t buffer_index,
                                            const VertexPositionT& v_pos,
                                            const VertexPositionT::PositionT::ValueT z) const noexcept {
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

void render::RendererLayer::SetBufferVertex(const uint32_t buffer_index,
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

void render::RendererLayer::SetBufferUv(const uint32_t buffer_index, const UvPositionT& u_pos) const noexcept {
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

std::unique_ptr<unsigned int[]> render::RendererLayer::GenRenderGridIndices(const uint32_t tile_count) {
    // Indices generation pattern:
    // top left
    // top right
    // bottom right
    // bottom left

    std::unique_ptr<unsigned int[]> positions(
        new unsigned int[core::SafeCast<uint64_t>(tile_count) * kInIndicesPerElement]);

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
