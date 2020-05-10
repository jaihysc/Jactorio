// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/12/2020

#include "renderer/rendering/renderer_layer.h"
#include "renderer/opengl/vertex_array.h"

// Settings

constexpr uint32_t kInitialSize        = 1;  // How many elements to reserve upon construction
constexpr uint32_t kIncrementOnResize = 300;  // How many more elements to reserve on resize

// Construct

jactorio::renderer::RendererLayer::RendererLayer() {
	Reserve(kInitialSize);
}

jactorio::renderer::RendererLayer::~RendererLayer() {
	DeleteBuffersS();

	// OpenGL buffers
	GDeleteBufferS();
}

// Set

void jactorio::renderer::RendererLayer::DeleteBuffersS() const {
	delete[] vertexBuffer_.ptr;
	delete[] uvBuffer_.ptr;
}

void jactorio::renderer::RendererLayer::PushBack(const Element& element) {
	if (nextElementIndex_ >= eCapacity_) {
		// No more space, resize
		Resize(eCapacity_ + kIncrementOnResize);
	}

	// Add
	const auto buffer_index = nextElementIndex_ * 8;
	SetBufferVertex(buffer_index, element.vertex);
	SetBufferUv(buffer_index, element.uv);
	nextElementIndex_++;
}

// If assertions enabled: Ensures element_index is in range. Also sets next_element_index_ for push_back to the highest
#define CHECK_ELEMENT_INDEX\
	assert(element_index < eCapacity_);\
	if (element_index >= nextElementIndex_)\
		nextElementIndex_ = element_index + 1;

void jactorio::renderer::RendererLayer::Set(const uint32_t element_index, const Element element) const {
	CHECK_ELEMENT_INDEX;

	const auto buffer_index = element_index * 8;
	SetBufferVertex(buffer_index, element.vertex);
	SetBufferUv(buffer_index, element.uv);
}

void jactorio::renderer::RendererLayer::SetVertex(const uint32_t element_index,
                                                  const core::QuadPosition element) const {
	CHECK_ELEMENT_INDEX;
	SetBufferVertex(element_index * 8, element);
}

void jactorio::renderer::RendererLayer::SetUv(const uint32_t element_index, const core::QuadPosition element) const {
	CHECK_ELEMENT_INDEX;
	SetBufferUv(element_index * 8, element);
}

#undef CHECK_ELEMENT_INDEX

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

	DeleteBuffersS();
	gResizeVertexBuffers_ = true;

	const auto floats = count * 8;  // 4 positions, of X and Y (8 floats in total)

	vertexBuffer_.ptr = new float[floats];
	uvBuffer_.ptr     = new float[floats];

	eCapacity_ = count;
}

void jactorio::renderer::RendererLayer::Resize(const uint32_t count) {
	// 4 step process to copying:
	// 1. Reserve new buffer
	// 2. copy existing buffer into new buffer
	// 3. delete old buffer
	// 4. hand pointers for new buffer off

	const auto floats       = count * 8;  // 4 positions, of X and Y (8 floats in total)
	auto* vertex_buffer_new = new float[floats];
	auto* uv_buffer_new     = new float[floats];

	// End for copying is the smallest element count between old and new size
	uint32_t end;
	if (eCapacity_ > count)
		end = count;
	else
		end = eCapacity_;

	end *= 8;  // 8 floats per element

	for (uint32_t i = 0; i < end; ++i) {
		vertex_buffer_new[i] = vertexBuffer_.ptr[i];
		uv_buffer_new[i]     = uvBuffer_.ptr[i];
	}

	DeleteBuffersS();
	gResizeVertexBuffers_ = true;

	vertexBuffer_.ptr = vertex_buffer_new;
	uvBuffer_.ptr     = uv_buffer_new;

	eCapacity_ = count;
}

uint32_t jactorio::renderer::RendererLayer::GetCapacity() const {
	return eCapacity_;
}

uint32_t jactorio::renderer::RendererLayer::GetElementCount() const {
	return nextElementIndex_;
}

void jactorio::renderer::RendererLayer::DeleteBuffer() noexcept {
	eCapacity_ = 0;

	DeleteBuffersS();

	vertexBuffer_.ptr = nullptr;
	uvBuffer_.ptr     = nullptr;
}

void jactorio::renderer::RendererLayer::Clear() const {
	nextElementIndex_ = 0;
}

// #######################################################################
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

		// const unsigned int start = x + y * tiles_x;
		//
		// positions[index++] = start;
		// positions[index++] = start + 1;
		// positions[index++] = start + 1 + tiles_x;
		//
		// positions[index++] = start + 1 + tiles_x;
		// positions[index++] = start + tiles_x;
		// positions[index++] = start;
	}

	return positions;

}

void jactorio::renderer::RendererLayer::GDeleteBufferS() const {
	delete vertexArray_;
	delete vertexVb_;
	delete uvVb_;
	delete indexIb_;
}

void jactorio::renderer::RendererLayer::GInitBuffer() {
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

void jactorio::renderer::RendererLayer::GUpdateData() {
	// Only resize if resize is set
	if (gResizeVertexBuffers_) {
		gResizeVertexBuffers_ = false;

		vertexVb_->Reserve(vertexBuffer_.ptr, eCapacity_ * kGByteMultiplier, false);
		uvVb_->Reserve(uvBuffer_.ptr, eCapacity_ * kGByteMultiplier, false);

		// Index buffer
		const auto* data = GenRenderGridIndices(eCapacity_);
		indexIb_->Reserve(data, eCapacity_ * 6);
		delete[] data;

		return;
	}

	// Since this is dealing in size, next_element_index_ is not subtracted by 1
	vertexVb_->UpdateData(vertexBuffer_.ptr, 0, nextElementIndex_ * kGByteMultiplier);
	uvVb_->UpdateData(uvBuffer_.ptr, 0, nextElementIndex_ * kGByteMultiplier);
}

void jactorio::renderer::RendererLayer::GDeleteBuffer() {
	GDeleteBufferS();

	vertexArray_ = nullptr;
	vertexVb_    = nullptr;
	uvVb_        = nullptr;
	indexIb_     = nullptr;
}

void jactorio::renderer::RendererLayer::GBufferBind() const {
	vertexArray_->Bind();
	vertexVb_->Bind();
	uvVb_->Bind();
	indexIb_->Bind();
}
