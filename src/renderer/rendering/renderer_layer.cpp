#include <renderer/rendering/renderer_grid.h>
#include "renderer/rendering/renderer_layer.h"
#include "renderer/opengl/vertex_array.h"

// Settings

constexpr uint32_t initial_size = 1;  // How many elements to reserve upon construction
constexpr uint32_t increment_on_resize = 300;  // How many more elements to reserve on resize

// Construct

jactorio::renderer::Renderer_layer::Renderer_layer() {
	reserve(initial_size);
}

jactorio::renderer::Renderer_layer::~Renderer_layer() {
	delete_buffers_s();

	// OpenGL buffers
	g_delete_buffer_s();
}

// Set

void jactorio::renderer::Renderer_layer::delete_buffers_s() const {
	delete[] vertex_buffer_.ptr;
	delete[] uv_buffer_.ptr;
}

void jactorio::renderer::Renderer_layer::push_back(const Element& element) {
	if (next_element_index_ >= e_capacity_) {
		// No more space, resize
		resize(e_capacity_ + increment_on_resize);
	}

	// Add
	const auto buffer_index = next_element_index_ * 8;
	set_buffer_vertex(buffer_index, element.vertex);
	set_buffer_uv(buffer_index, element.uv);
	next_element_index_++;
}

// If assertions enabled: Ensures element_index is in range. Also sets next_element_index_ for push_back to the highest
#define CHECK_ELEMENT_INDEX\
	assert(element_index < e_capacity_);\
	if (element_index >= next_element_index_)\
		next_element_index_ = element_index + 1;

void jactorio::renderer::Renderer_layer::set(const uint32_t element_index, const Element element) const {
	CHECK_ELEMENT_INDEX;

	const auto buffer_index = element_index * 8;
	set_buffer_vertex(buffer_index, element.vertex);
	set_buffer_uv(buffer_index, element.uv);
}

void jactorio::renderer::Renderer_layer::set_vertex(const uint32_t element_index, const core::Quad_position element) const {
	CHECK_ELEMENT_INDEX;
	set_buffer_vertex(element_index * 8, element);
}

void jactorio::renderer::Renderer_layer::set_uv(const uint32_t element_index, const core::Quad_position element) const {
	CHECK_ELEMENT_INDEX;
	set_buffer_uv(element_index * 8, element);
}

#undef CHECK_ELEMENT_INDEX

void jactorio::renderer::Renderer_layer::set_buffer_vertex(const uint32_t buffer_index,
                                                           const core::Quad_position& element) const {
	// Populate in following order: topL, topR, bottomR, bottomL (X Y)

	// Vertex
	vertex_buffer_.ptr[buffer_index + 0] = element.top_left.x;
	vertex_buffer_.ptr[buffer_index + 1] = element.top_left.y;


	vertex_buffer_.ptr[buffer_index + 2] = element.bottom_right.x;
	vertex_buffer_.ptr[buffer_index + 3] = element.top_left.y;

	vertex_buffer_.ptr[buffer_index + 4] = element.bottom_right.x;
	vertex_buffer_.ptr[buffer_index + 5] = element.bottom_right.y;

	vertex_buffer_.ptr[buffer_index + 6] = element.top_left.x;
	vertex_buffer_.ptr[buffer_index + 7] = element.bottom_right.y;
}

void jactorio::renderer::Renderer_layer::set_buffer_uv(const uint32_t buffer_index, 
                                                       const core::Quad_position& element) const {
	// Populate in following order: bottomL, bottomR, topR, topL (X Y) (NOT THE SAME AS ABOVE!!)

	// UV
	uv_buffer_.ptr[buffer_index + 0] = element.top_left.x;
	uv_buffer_.ptr[buffer_index + 1] = element.bottom_right.y;

	uv_buffer_.ptr[buffer_index + 2] = element.bottom_right.x;
	uv_buffer_.ptr[buffer_index + 3] = element.bottom_right.y;

	uv_buffer_.ptr[buffer_index + 4] = element.bottom_right.x;
	uv_buffer_.ptr[buffer_index + 5] = element.top_left.y;

	uv_buffer_.ptr[buffer_index + 6] = element.top_left.x;
	uv_buffer_.ptr[buffer_index + 7] = element.top_left.y;
}


// Get buffers

jactorio::renderer::Renderer_layer::Buffer<float> jactorio::renderer::Renderer_layer::get_buf_vertex() {
	vertex_buffer_.count = e_capacity_;
	return vertex_buffer_;
}

jactorio::renderer::Renderer_layer::Buffer<float> jactorio::renderer::Renderer_layer::get_buf_uv() {
	uv_buffer_.count = e_capacity_;
	return uv_buffer_;
}

const jactorio::renderer::Index_buffer* jactorio::renderer::Renderer_layer::get_buf_index() const {
	return index_ib_;
}


// Resizing

void jactorio::renderer::Renderer_layer::reserve(const uint32_t count) {
	assert(count > 0);  // Count must be greater than 0
	
	delete_buffers_s();
	g_resize_vertex_buffers_ = true;

	const auto floats = count * 8;  // 4 positions, of X and Y (8 floats in total)
	
	vertex_buffer_.ptr = new float[floats];
	uv_buffer_.ptr = new float[floats];

	e_capacity_ = count;
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
	if (e_capacity_ > count)
		end = count;
	else
		end = e_capacity_;
	
	end *= 8;  // 8 floats per element
	
	for (uint32_t i = 0; i < end; ++i) {
		vertex_buffer_new[i] = vertex_buffer_.ptr[i];
		uv_buffer_new[i] = uv_buffer_.ptr[i];
	}

	delete_buffers_s();
	g_resize_vertex_buffers_ = true;

	vertex_buffer_.ptr = vertex_buffer_new;
	uv_buffer_.ptr = uv_buffer_new;

	e_capacity_ = count;
}

uint32_t jactorio::renderer::Renderer_layer::get_capacity() const {
	return e_capacity_;
}

uint32_t jactorio::renderer::Renderer_layer::get_element_count() const {
	return next_element_index_;
}

void jactorio::renderer::Renderer_layer::delete_buffer() noexcept {
	e_capacity_ = 0;
	
	delete_buffers_s();

	vertex_buffer_.ptr = nullptr;
	uv_buffer_.ptr = nullptr;
}

void jactorio::renderer::Renderer_layer::clear() const {
	next_element_index_ = 0;
}

// #######################################################################
// OpenGL methods
constexpr uint64_t g_byte_multiplier = 8 * sizeof(float);  // Multiply by this to convert to bytes

void jactorio::renderer::Renderer_layer::g_delete_buffer_s() const {
	delete vertex_array_;
	delete vertex_vb_;
	delete uv_vb_;
	delete index_ib_;
}

void jactorio::renderer::Renderer_layer::g_init_buffer() {
	vertex_vb_ = new Vertex_buffer(vertex_buffer_.ptr, e_capacity_ * g_byte_multiplier, false);
	uv_vb_ = new Vertex_buffer(uv_buffer_.ptr, e_capacity_ * g_byte_multiplier, false);

	vertex_array_ = new Vertex_array();
	// Register buffer properties and shader location
	// Vertex - location 0
	// UV - location 1
	vertex_array_->add_buffer(vertex_vb_, 2, 0);
	vertex_array_->add_buffer(uv_vb_, 2, 1);

	// Index buffer
	// As of right now there is no point holding onto the index_buffer data pointer after handing it to the GPu
	// since it is never modified
	const auto data = renderer_grid::gen_render_grid_indices(e_capacity_);
	index_ib_ = new Index_buffer(data, e_capacity_ * 6);
	delete[] data;

	// No need to resize anymore
	g_resize_vertex_buffers_ = false;
}

void jactorio::renderer::Renderer_layer::g_update_data() {
	// Only resize if resize is set
	if (g_resize_vertex_buffers_) {
		g_resize_vertex_buffers_ = false;

		vertex_vb_->reserve(vertex_buffer_.ptr, e_capacity_ * g_byte_multiplier, false);
		uv_vb_->reserve(uv_buffer_.ptr, e_capacity_ * g_byte_multiplier, false);

		// Index buffer
		const auto data = renderer_grid::gen_render_grid_indices(e_capacity_);
		index_ib_->reserve(data, e_capacity_ * 6);
		delete[] data;

		return;
	}

	// Since this is dealing in size, next_element_index_ is not subtracted by 1
	vertex_vb_->update_data(vertex_buffer_.ptr, 0, next_element_index_ * g_byte_multiplier);
	uv_vb_->update_data(uv_buffer_.ptr, 0, next_element_index_ * g_byte_multiplier);
}

void jactorio::renderer::Renderer_layer::g_delete_buffer() {
	g_delete_buffer_s();

	vertex_array_ = nullptr;
	vertex_vb_ = nullptr;
	uv_vb_ = nullptr;
	index_ib_ = nullptr;
}

void jactorio::renderer::Renderer_layer::g_buffer_bind() const {
	vertex_array_->bind();
	vertex_vb_->bind();
	uv_vb_->bind();
	index_ib_->bind();
}
