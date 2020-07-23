// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/rendering/renderer_layer.h"
#include "renderer/opengl/vertex_array.h"

using namespace jactorio;

renderer::RendererLayer::RendererLayer() {
	GlInitBuffers();
	Reserve(kInitialSize);
	GlHandleBufferResize();
}

renderer::RendererLayer::~RendererLayer() {
	GlFreeBuffers();
}

// ======================================================================

void renderer::RendererLayer::PushBack(const Element& element) {
	assert(writeEnabled_);

	// Skips data appending operation if buffer needs to resize
	if (gResizeVertexBuffers_) {
		++queuedECapacity_;  // Will reserve an additional element for this one that was skipped
		return;
	}

	if (nextElementIndex_ >= eCapacity_) {
		// No more space, reserve, will be repopulated on next update
		Reserve(eCapacity_ + 1);
		return;
	}

	// Add
	const auto buffer_index = nextElementIndex_ * kVbIndicesPerElement;
	SetBufferVertex(buffer_index, element.vertex);
	SetBufferUv(buffer_index, element.uv);
	nextElementIndex_++;
}

uint32_t renderer::RendererLayer::GetCapacity() const noexcept {
	return eCapacity_;
}

uint64_t renderer::RendererLayer::GetIndicesCount() const {
	return static_cast<uint64_t>(nextElementIndex_) * kInIndicesPerElement;
}

void renderer::RendererLayer::Reserve(const uint32_t count) {
	assert(count > 0);  // Count must be greater than 0

	gResizeVertexBuffers_ = true;
	queuedECapacity_      = count;
	LOG_MESSAGE(debug, "Queuing buffer resize");
}

void renderer::RendererLayer::ResizeDefault() {
	Reserve(kInitialSize);
}

void renderer::RendererLayer::Clear() {
	nextElementIndex_ = 0;
}

void renderer::RendererLayer::GlWriteBegin() {
	assert(!writeEnabled_);
	assert(eCapacity_ > 0);  // Mapping fails unless capacity is at least 1

	vertexBuffer_ = static_cast<float*>(vertexVb_->Map());
	uvBuffer_     = static_cast<float*>(uvVb_->Map());

	writeEnabled_ = true;
}

void renderer::RendererLayer::GlWriteEnd() {
	assert(writeEnabled_);

	vertexVb_->UnMap();
	uvVb_->UnMap();

	writeEnabled_ = false;
}

void renderer::RendererLayer::GlHandleBufferResize() {
	// Only resize if resize is set
	if (!gResizeVertexBuffers_)
		return;

	gResizeVertexBuffers_ = false;
	eCapacity_            = queuedECapacity_ * kResizeECapacityMultiplier;

	vertexVb_->Reserve(nullptr, eCapacity_ * kVbBytesPerElement, false);
	uvVb_->Reserve(nullptr, eCapacity_ * kVbBytesPerElement, false);

	// Index buffer
	const auto data = GenRenderGridIndices(eCapacity_);
	indexIb_->Reserve(data.get(), eCapacity_ * kInIndicesPerElement);

	LOG_MESSAGE_F(debug, "Buffer resized to %d", eCapacity_);
}

void renderer::RendererLayer::GlDeleteBuffers() {
	GlFreeBuffers();

	vertexArray_ = nullptr;
	vertexVb_    = nullptr;
	uvVb_        = nullptr;
	indexIb_     = nullptr;
}

void renderer::RendererLayer::GlBindBuffers() const {
	vertexArray_->Bind();
	vertexVb_->Bind();
	uvVb_->Bind();
	indexIb_->Bind();
}

// ======================================================================

void renderer::RendererLayer::SetBufferVertex(const uint32_t buffer_index,
                                              const VertexPositionT& element) const {
	assert(element.topLeft.z == element.bottomRight.z);
	
	// Populate in following order: topL, topR, bottomR, bottomL (X Y)

	vertexBuffer_[buffer_index + 0] = element.topLeft.x;
	vertexBuffer_[buffer_index + 1] = element.topLeft.y;
	vertexBuffer_[buffer_index + 2] = element.topLeft.z;

	vertexBuffer_[buffer_index + 3] = element.bottomRight.x;
	vertexBuffer_[buffer_index + 4] = element.topLeft.y;
	vertexBuffer_[buffer_index + 5] = element.topLeft.z;

	vertexBuffer_[buffer_index + 6] = element.bottomRight.x;
	vertexBuffer_[buffer_index + 7] = element.bottomRight.y;
	vertexBuffer_[buffer_index + 8] = element.topLeft.z;

	vertexBuffer_[buffer_index + 9]  = element.topLeft.x;
	vertexBuffer_[buffer_index + 10] = element.bottomRight.y;
	vertexBuffer_[buffer_index + 11] = element.topLeft.z;
}

void renderer::RendererLayer::SetBufferUv(const uint32_t buffer_index,
                                          const UvPositionT& element) const {
	// Populate in following order: bottomL, bottomR, topR, topL (X Y) (NOT THE SAME AS ABOVE!!)

	// UV
	uvBuffer_[buffer_index + 0] = element.topLeft.x;
	uvBuffer_[buffer_index + 1] = element.bottomRight.y;

	uvBuffer_[buffer_index + 3] = element.bottomRight.x;
	uvBuffer_[buffer_index + 4] = element.bottomRight.y;

	uvBuffer_[buffer_index + 6] = element.bottomRight.x;
	uvBuffer_[buffer_index + 7] = element.topLeft.y;

	uvBuffer_[buffer_index + 9] = element.topLeft.x;
	uvBuffer_[buffer_index + 10] = element.topLeft.y;
}

std::unique_ptr<unsigned int[]> renderer::RendererLayer::GenRenderGridIndices(const uint32_t tile_count) {
	// Indices generation pattern:
	// top left
	// top right
	// bottom right
	// bottom left

	std::unique_ptr<unsigned int[]> positions(
		new unsigned int[static_cast<uint64_t>(tile_count) * kInIndicesPerElement]
	);

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

void renderer::RendererLayer::GlInitBuffers() {
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

void renderer::RendererLayer::GlFreeBuffers() const {
	delete vertexArray_;
	delete vertexVb_;
	delete uvVb_;
	delete indexIb_;
}
