// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/rendering/renderer_layer.h"
#include "renderer/opengl/vertex_array.h"

// Settings

constexpr uint32_t kInitialSize = 1;  // How many elements to reserve upon construction

// Construct

jactorio::renderer::RendererLayer::RendererLayer() {
	Reserve(kInitialSize);
}

jactorio::renderer::RendererLayer::~RendererLayer() {
	GlFreeBuffers();
}

// Set

void jactorio::renderer::RendererLayer::PushBack(const Element& element) {
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
	const auto buffer_index = nextElementIndex_ * 8;
	SetBufferVertex(buffer_index, element.vertex);
	SetBufferUv(buffer_index, element.uv);
	nextElementIndex_++;
}

void jactorio::renderer::RendererLayer::SetBufferVertex(const uint32_t buffer_index,
                                                        const core::QuadPosition& element) const {
	// Populate in following order: topL, topR, bottomR, bottomL (X Y)

	// Vertex
	vertexBuffer_.ptr[buffer_index + 0] = element.topLeft.x;
	vertexBuffer_.ptr[buffer_index + 1] = element.topLeft.y;


	vertexBuffer_.ptr[buffer_index + 2] = element.bottomRight.x;
	vertexBuffer_.ptr[buffer_index + 3] = element.topLeft.y;

	vertexBuffer_.ptr[buffer_index + 4] = element.bottomRight.x;
	vertexBuffer_.ptr[buffer_index + 5] = element.bottomRight.y;

	vertexBuffer_.ptr[buffer_index + 6] = element.topLeft.x;
	vertexBuffer_.ptr[buffer_index + 7] = element.bottomRight.y;
}

void jactorio::renderer::RendererLayer::SetBufferUv(const uint32_t buffer_index,
                                                    const core::QuadPosition& element) const {
	// Populate in following order: bottomL, bottomR, topR, topL (X Y) (NOT THE SAME AS ABOVE!!)

	// UV
	uvBuffer_.ptr[buffer_index + 0] = element.topLeft.x;
	uvBuffer_.ptr[buffer_index + 1] = element.bottomRight.y;

	uvBuffer_.ptr[buffer_index + 2] = element.bottomRight.x;
	uvBuffer_.ptr[buffer_index + 3] = element.bottomRight.y;

	uvBuffer_.ptr[buffer_index + 4] = element.bottomRight.x;
	uvBuffer_.ptr[buffer_index + 5] = element.topLeft.y;

	uvBuffer_.ptr[buffer_index + 6] = element.topLeft.x;
	uvBuffer_.ptr[buffer_index + 7] = element.topLeft.y;
}


// Get buffers

jactorio::renderer::RendererLayer::Buffer<float> jactorio::renderer::RendererLayer::GetBufVertex() {
	vertexBuffer_.count = eCapacity_;
	return vertexBuffer_;
}

jactorio::renderer::RendererLayer::Buffer<float> jactorio::renderer::RendererLayer::GetBufUv() {
	uvBuffer_.count = eCapacity_;
	return uvBuffer_;
}

const jactorio::renderer::IndexBuffer* jactorio::renderer::RendererLayer::GetBufIndex() const {
	return indexIb_;
}


// Resizing

void jactorio::renderer::RendererLayer::Reserve(const uint32_t count) {
	assert(count > 0);  // Count must be greater than 0

	gResizeVertexBuffers_ = true;
	queuedECapacity_      = count;
	LOG_MESSAGE(debug, "Queuing buffer resize");
}

void jactorio::renderer::RendererLayer::Clear() const {
	nextElementIndex_ = 0;
}

// ======================================================================
// OpenGL methods
constexpr uint64_t kGByteMultiplier = 8 * sizeof(float);  // Multiply by this to convert to bytes

unsigned* jactorio::renderer::RendererLayer::GenRenderGridIndices(const uint32_t tile_count) {
	// Indices generation pattern:
	// top left
	// top right
	// bottom right
	// bottom left

	auto* positions = new unsigned int[static_cast<uint64_t>(tile_count) * 6];

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

void jactorio::renderer::RendererLayer::GlFreeBuffers() const {
	delete vertexArray_;
	delete vertexVb_;
	delete uvVb_;
	delete indexIb_;
}

void jactorio::renderer::RendererLayer::GlInitBuffers() {
	vertexVb_ = new VertexBuffer(vertexBuffer_.ptr, eCapacity_ * kGByteMultiplier, false);
	uvVb_     = new VertexBuffer(uvBuffer_.ptr, eCapacity_ * kGByteMultiplier, false);

	vertexArray_ = new VertexArray();
	// Register buffer properties and shader location
	// Vertex - location 0
	// UV - location 1
	vertexArray_->AddBuffer(vertexVb_, 2, 0);
	vertexArray_->AddBuffer(uvVb_, 2, 1);

	// Index buffer
	// As of right now there is no point holding onto the index_buffer data pointer after handing it to the GPu
	// since it is never modified
	const auto* data = GenRenderGridIndices(eCapacity_);
	indexIb_         = new IndexBuffer(data, eCapacity_ * 6);
	delete[] data;

	// No need to resize anymore
	gResizeVertexBuffers_ = false;
}

void jactorio::renderer::RendererLayer::GlWriteBegin() {
	assert(!writeEnabled_);

	vertexBuffer_.ptr = static_cast<float*>(vertexVb_->Map());
	uvBuffer_.ptr     = static_cast<float*>(uvVb_->Map());

	writeEnabled_ = true;
}

void jactorio::renderer::RendererLayer::GlWriteEnd() {
	assert(writeEnabled_);

	vertexVb_->UnMap();
	uvVb_->UnMap();

	writeEnabled_ = false;
}

void jactorio::renderer::RendererLayer::GlHandleBufferResize() {
	// Only resize if resize is set
	if (!gResizeVertexBuffers_)
		return;

	gResizeVertexBuffers_ = false;
	eCapacity_            = queuedECapacity_;

	vertexVb_->Reserve(nullptr, eCapacity_ * kGByteMultiplier, false);
	uvVb_->Reserve(nullptr, eCapacity_ * kGByteMultiplier, false);

	// Index buffer
	const auto* data = GenRenderGridIndices(eCapacity_);
	indexIb_->Reserve(data, eCapacity_ * 6);
	delete[] data;

	LOG_MESSAGE_F(debug, "Buffer resized to %d", eCapacity_);
}

void jactorio::renderer::RendererLayer::GlDeleteBuffers() {
	GlFreeBuffers();

	vertexArray_ = nullptr;
	vertexVb_    = nullptr;
	uvVb_        = nullptr;
	indexIb_     = nullptr;
}

void jactorio::renderer::RendererLayer::GlBindBuffers() const {
	vertexArray_->Bind();
	vertexVb_->Bind();
	uvVb_->Bind();
	indexIb_->Bind();
}
