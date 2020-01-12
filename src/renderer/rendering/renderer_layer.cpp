#include "renderer/rendering/renderer_layer.h"

// Settings

constexpr uint32_t initial_size = 1;  // How many elements to reserve upon construction
constexpr uint32_t increment_on_resize = 1;  // How many more elements to reserve on resize

// Construct

jactorio::renderer::Renderer_layer::Renderer_layer(const bool static_layer)
	: static_layer_(static_layer) {
	reserve(initial_size);
}

jactorio::renderer::Renderer_layer::~Renderer_layer() {
	delete_buffers_s();
}

// Set

void jactorio::renderer::Renderer_layer::delete_buffers_s() const {
	delete[] vertex_buffer_.ptr;
	delete[] uv_buffer_.ptr;
}

void jactorio::renderer::Renderer_layer::push_back(const Element element) {
	if (next_element_index_ >= e_count_) {
		// No more space, resize
		assert(!static_layer_);  // Cannot resize static layer
		
		resize(e_count_ + increment_on_resize);
		next_element_index_ = 0;
	}
	
	// Add
	set(next_element_index_++, element);
}

void jactorio::renderer::Renderer_layer::set(const uint32_t element_index, const Element element) const {
	const auto buffer_index = element_index * 8;

	// Populate in following order: bottomL, bottomR, topR, topL (X Y)
	
	// Vertex
	vertex_buffer_.ptr[buffer_index + 0] = element.vertex.top_left.x;
	vertex_buffer_.ptr[buffer_index + 1] = element.vertex.bottom_right.y;

	vertex_buffer_.ptr[buffer_index + 2] = element.vertex.bottom_right.x;
	vertex_buffer_.ptr[buffer_index + 3] = element.vertex.bottom_right.y;

	vertex_buffer_.ptr[buffer_index + 4] = element.vertex.bottom_right.x;
	vertex_buffer_.ptr[buffer_index + 5] = element.vertex.top_left.y;

	vertex_buffer_.ptr[buffer_index + 6] = element.vertex.top_left.x;
	vertex_buffer_.ptr[buffer_index + 7] = element.vertex.top_left.y;

	// UV
	uv_buffer_.ptr[buffer_index + 0] = element.uv.top_left.x;
	uv_buffer_.ptr[buffer_index + 1] = element.uv.bottom_right.y;

	uv_buffer_.ptr[buffer_index + 2] = element.uv.bottom_right.x;
	uv_buffer_.ptr[buffer_index + 3] = element.uv.bottom_right.y;

	uv_buffer_.ptr[buffer_index + 4] = element.uv.bottom_right.x;
	uv_buffer_.ptr[buffer_index + 5] = element.uv.top_left.y;

	uv_buffer_.ptr[buffer_index + 6] = element.uv.top_left.x;
	uv_buffer_.ptr[buffer_index + 7] = element.uv.top_left.y;
}


// Get buffers

jactorio::renderer::Renderer_layer::Buffer<float> jactorio::renderer::Renderer_layer::get_buf_vertex() {
	vertex_buffer_.count = e_count_;
	return vertex_buffer_;
}

jactorio::renderer::Renderer_layer::Buffer<float> jactorio::renderer::Renderer_layer::get_buf_uv() {
	uv_buffer_.count = e_count_;
	return uv_buffer_;
}


// Resizing

void jactorio::renderer::Renderer_layer::reserve(const uint32_t count) {
	assert(count > 0);  // Count must be greater than 0
	
	delete_buffers_s();
	
	const auto floats = count * 8;  // 4 positions, of X and Y (8 floats in total)
	
	vertex_buffer_.ptr = new float[floats];
	uv_buffer_.ptr = new float[floats];

	e_count_ = count;
}

void jactorio::renderer::Renderer_layer::resize(const uint32_t count) {
	// 4 step process to copying:
	// 1. Reserve new buffer
	// 2. copy existing buffer into new buffer
	// 3. delete old buffer
	// 4. hand pointers for new buffer off
	
	const auto floats = count * 8;  // 4 positions, of X and Y (8 floats in total)
	auto* vertex_buffer_new = new float[floats];
	auto* uv_buffer_new = new float[floats];

	// End for copying is the smallest element count between old and new size
	uint32_t end;
	if (e_count_ > count)
		end = count;
	else
		end = e_count_;
	
	end *= 8;  // 8 floats per element
	
	for (uint32_t i = 0; i < end; ++i) {
		vertex_buffer_new[i] = vertex_buffer_.ptr[i];
		uv_buffer_new[i] = uv_buffer_.ptr[i];
	}

	delete_buffers_s();

	vertex_buffer_.ptr = vertex_buffer_new;
	uv_buffer_.ptr = uv_buffer_new;

	e_count_ = count;
}

uint32_t jactorio::renderer::Renderer_layer::e_count() const {
	return e_count_;
}

void jactorio::renderer::Renderer_layer::delete_buffer() noexcept {
	e_count_ = 0;
	
	delete_buffers_s();

	vertex_buffer_.ptr = nullptr;
	uv_buffer_.ptr = nullptr;
}
