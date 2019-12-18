#include <GL/glew.h>

#include "renderer/rendering/renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/renderer_grid.h"
#include "renderer/rendering/mvp_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/error.h"
#include "data/data_manager.h"

unsigned short jactorio::renderer::Renderer::window_width_ = 0;
unsigned short jactorio::renderer::Renderer::window_height_ = 0;

unsigned short jactorio::renderer::Renderer::tile_width = 9;

std::unordered_map<unsigned int, jactorio::renderer::Renderer_sprites::Image_position>
jactorio::renderer::Renderer::spritemap_coords_{};

void jactorio::renderer::Renderer::set_spritemap_coords(
	const std::unordered_map<unsigned, Renderer_sprites::Image_position>& spritemap_coords) {
	spritemap_coords_ = spritemap_coords;
}

jactorio::renderer::Renderer_sprites::Image_position jactorio::renderer::Renderer::
get_spritemap_coords(const unsigned internal_id) {
	return spritemap_coords_.at(internal_id);
}


// non static
void jactorio::renderer::Renderer::update_tile_projection_matrix() {
    // FIXME the tile_width does not at times correspond to the actual tile width with the proj matrix enabled

    const auto max_tile_width = static_cast<float>(tile_width * 2);

	 if (tile_projection_matrix_offset < max_tile_width)
	 	// Prevent zooming out too far
	 	tile_projection_matrix_offset = max_tile_width;
	 else {
	 	// Prevent zooming too far in
	 	unsigned short smallest_axis;
	 	if (window_width_ > window_height_) {
	 		smallest_axis = window_height_;
	 	}
	 	else {
	 		smallest_axis = window_width_;
	 	}

	 	// Maximum zoom is 30 from center
	 	const int max_zoom_offset = 30;
	 	if (tile_projection_matrix_offset > static_cast<float>(smallest_axis) / 2 - max_zoom_offset) {
	 		tile_projection_matrix_offset = static_cast<float>(smallest_axis) / 2 - max_zoom_offset;
	 	}
	 }

	setg_projection_matrix(
		mvp_manager::to_proj_matrix(window_width_, window_height_, tile_projection_matrix_offset)
	);
}

jactorio::renderer::Renderer::Renderer() {
	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	recalculate_buffers(m_viewport[2], m_viewport[3]);
}

jactorio::renderer::Renderer::~Renderer() {
	delete_data();
}

void jactorio::renderer::Renderer::recalculate_buffers(const unsigned short window_x,
                                                       const unsigned short window_y) {
	delete_data();

	window_width_ = window_x;
	window_height_ = window_y;
	update_tile_projection_matrix();


	const mvp_manager::Projection_tile_data tile_projection_data =
		mvp_manager::projection_calculate_tile_properties(tile_width, window_x, window_y);

	// Initialize fields
	// Raise the bottom and right by tile_width so the last tile has enough space to render out
	tile_count_x_ = tile_projection_data.tiles_x + 1;
	tile_count_y_ = tile_projection_data.tiles_y + 1;

	grid_vertices_count_ = (tile_count_x_ + 1) * (tile_count_y_ + 1);
	grid_elements_count_ = tile_count_x_ * tile_count_y_;


	// Initialization of vertex array and its buffers
	vertex_array_ = new Vertex_array();

	// Render grid
	{
		const auto data = renderer_grid::gen_render_tile_grid(
			tile_count_x_,
			tile_count_y_,
			tile_width);

		// Size of 1 element: 8 * sizeof(float)
		render_grid_ = new Vertex_buffer(data, grid_elements_count_ * 4 * 2 * sizeof(float));
		vertex_array_->add_buffer(*render_grid_, 2, 0);
		delete[] data;
	}

	// Spritemap positions
	{
		const auto data = renderer_grid::gen_texture_grid(grid_elements_count_);

		texture_grid_ = new Vertex_buffer(data, grid_elements_count_ * 4 * 2 * sizeof(float));
		vertex_array_->add_buffer(*texture_grid_, 2, 1);
		delete[] data;
	}
	texture_grid_buffer_ = new float[
		static_cast<unsigned long long>(tile_count_x_) * tile_count_y_ * 8];


	// Index buffer
	{
		const auto data = renderer_grid::gen_render_grid_indices(
			tile_count_x_,
			tile_count_y_
		);

		index_buffer_ = new Index_buffer(data, grid_elements_count_ * 6);
		delete[] data;
	}
}

void jactorio::renderer::Renderer::delete_data() const {
	delete vertex_array_;
	delete render_grid_;
	delete texture_grid_;
	delete index_buffer_;
	delete[] texture_grid_buffer_;
}


void jactorio::renderer::Renderer::draw(const glm::vec3 transform) const {
	vertex_array_->bind();
	index_buffer_->bind();

	const glm::mat4 model_matrix = translate(glm::mat4(1.f), transform);
	setg_model_matrix(model_matrix);
	update_shader_mvp();

	DEBUG_OPENGL_CALL(
		glDrawElements(GL_TRIANGLES, index_buffer_->count(), GL_UNSIGNED_INT, nullptr)
	); // Pointer not needed as buffer is already bound
}

void jactorio::renderer::Renderer::clear() {
	DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}


// Grid properties
unsigned short jactorio::renderer::Renderer::get_window_width() {
	return window_width_;
}

unsigned short jactorio::renderer::Renderer::get_window_height() {
	return window_height_;
}

float* jactorio::renderer::Renderer::get_texture_grid_buffer() const {
	return texture_grid_buffer_;
}

void jactorio::renderer::Renderer::update_texture_grid_buffer() const {
	texture_grid_->set_buffer_data(texture_grid_buffer_, 0,
	                               tile_count_x_ * tile_count_y_ * sizeof(float) * 8);
}

unsigned short jactorio::renderer::Renderer::get_grid_size_x() const {
	return tile_count_x_;
}

unsigned short jactorio::renderer::Renderer::get_grid_size_y() const {
	return tile_count_y_;
}