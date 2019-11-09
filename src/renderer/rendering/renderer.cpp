#include <GL/glew.h>

#include "renderer/rendering/renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/renderer_grid.h"
#include "renderer/rendering/mvp_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/error.h"
#include "data/data_manager.h"


float jactorio::renderer::Renderer::tile_projection_matrix_offset = 0;


const unsigned short tile_width = 16;


unsigned short jactorio::renderer::Renderer::window_width_ = 10;
unsigned short jactorio::renderer::Renderer::window_height_ = 10;


void jactorio::renderer::Renderer::update_tile_projection_matrix() {
	// TODO There will be a black bar on the right and bottom from the tile not being wide enough to fit.
	// Possible fixes:
	// 1. Zooming in so one cannot see the black bar
	// 2. Extending the projection matrix area to render past the edge of the screen

	setg_projection_matrix(
		mvp_manager::to_proj_matrix(window_width_, window_height_, tile_projection_matrix_offset)
	);
}


jactorio::renderer::Renderer::Renderer(
	const std::unordered_map<std::string, Renderer_sprites::Image_position>& spritemap_coords) {
	spritemap_coords_ = spritemap_coords;
	
	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	recalculate_buffers(m_viewport[2], m_viewport[3]);
}

void jactorio::renderer::Renderer::recalculate_buffers(const unsigned short window_x,
                                                       const unsigned short window_y) {
	delete vertex_array_;
	delete render_grid_;
	delete texture_grid_;
	delete index_buffer_;

	window_width_ = window_x;
	window_height_ = window_y;
	update_tile_projection_matrix();


	const mvp_manager::Projection_tile_data tile_projection_data =
		mvp_manager::projection_calculate_tile_properties(tile_width, window_x, window_y);

	// Initialize fields
	tile_count_x_ = tile_projection_data.tiles_x;
	tile_count_y_ = tile_projection_data.tiles_y;
	
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

	
	// Index buffer
	{
		const auto data = renderer_grid::gen_render_grid_indices(
			tile_count_x_,
			tile_count_y_
		);
		
		index_buffer_ = new Index_buffer(data,grid_elements_count_ * 6);
		delete[] data;
	}
}

// Get all textures
// Concat into spritemap
// Spritemap is parameter of constructor
void jactorio::renderer::Renderer::set_sprite(const unsigned short index_x,
                                              const unsigned short index_y,
                                              const std::string& sprite_iname) {
	const auto data = new float[8];

	// TODO validate sprite iname exists?
	const auto sprite_coords = spritemap_coords_[sprite_iname];
	data[0] = sprite_coords.bottom_left.x;
	data[1] = sprite_coords.bottom_left.y, // bottom left
	
	data[2] = sprite_coords.bottom_right.x;
	data[3] = sprite_coords.bottom_right.y; // bottom right
	
	data[4] = sprite_coords.top_right.x;
	data[5] = sprite_coords.top_right.y; // upper right
	
	data[6] = sprite_coords.top_left.x;
	data[7] = sprite_coords.top_left.y; // upper left
	
	const unsigned int offset = (index_y * tile_count_x_ + index_x) * 8 * sizeof(float);
	
	texture_grid_->set_buffer_data(data, offset, sizeof(float) * 8);

	delete[] data;
}


jactorio::renderer::Renderer::~Renderer() {
	delete vertex_array_;
	delete render_grid_;
	delete texture_grid_;
	delete index_buffer_;
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
