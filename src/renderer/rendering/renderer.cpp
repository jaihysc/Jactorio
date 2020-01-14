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

std::unordered_map<unsigned int, jactorio::core::Quad_position> jactorio::renderer::Renderer::spritemap_coords_{};

void jactorio::renderer::Renderer::set_spritemap_coords(
	const std::unordered_map<unsigned, core::Quad_position>& spritemap_coords) {
	spritemap_coords_ = spritemap_coords;
}

jactorio::core::Quad_position jactorio::renderer::Renderer::get_spritemap_coords(const unsigned internal_id) {
	return spritemap_coords_.at(internal_id);
}


// non static
void jactorio::renderer::Renderer::update_tile_projection_matrix() {
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

	// Render layer
	render_layer = new Renderer_layer(true);
	render_layer->reserve(grid_elements_count_);
	render_layer->g_init_buffer();

	// Vertex positions
	renderer_grid::gen_render_grid(render_layer, tile_count_x_, tile_count_y_, tile_width);
}

void jactorio::renderer::Renderer::delete_data() const {
	delete vertex_array_;
	delete render_layer;
}


void jactorio::renderer::Renderer::g_draw(const glm::vec3 transform) const {
	vertex_array_->bind();

	const glm::mat4 model_matrix = translate(glm::mat4(1.f), transform);
	setg_model_matrix(model_matrix);
	update_shader_mvp();

	DEBUG_OPENGL_CALL(
		glDrawElements(GL_TRIANGLES, render_layer->get_buf_index()->count(), GL_UNSIGNED_INT, nullptr)
	); // Pointer not needed as buffer is already bound
}

void jactorio::renderer::Renderer::g_clear() {
	DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}


// Grid properties
unsigned short jactorio::renderer::Renderer::get_window_width() {
	return window_width_;
}

unsigned short jactorio::renderer::Renderer::get_window_height() {
	return window_height_;
}


unsigned short jactorio::renderer::Renderer::get_grid_size_x() const {
	return tile_count_x_;
}

unsigned short jactorio::renderer::Renderer::get_grid_size_y() const {
	return tile_count_y_;
}