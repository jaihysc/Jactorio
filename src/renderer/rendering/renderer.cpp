#include <GL/glew.h>

#include "renderer/rendering/renderer.h"

#include <memory>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/renderer_grid.h"
#include "renderer/rendering/mvp_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/error.h"


jactorio::renderer::mvp_manager::Projection_tile_data
	jactorio::renderer::Renderer::tile_projection_data_ = mvp_manager::Projection_tile_data{};
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


jactorio::renderer::Renderer::Renderer() {
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

	
	tile_projection_data_ =
		mvp_manager::projection_calculate_tile_properties(tile_width, window_x, window_y);

	grid_vertices_count_ = (tile_projection_data_.tiles_x + 1) * (tile_projection_data_.tiles_y + 1);
	grid_elements_count_ = tile_projection_data_.tiles_x * tile_projection_data_.tiles_y;

	
	// Initialization of vertex array and its buffers
	vertex_array_ = new Vertex_array();

	// Render grid
	{
		const auto data = renderer_grid::gen_render_tile_grid(
			tile_projection_data_.tiles_x,
			tile_projection_data_.tiles_y,
			tile_width);

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
			tile_projection_data_.tiles_x,
			tile_projection_data_.tiles_y
		);
		
		index_buffer_ = new Index_buffer(data,grid_elements_count_ * 6);
		delete[] data;
	}
}


jactorio::renderer::Renderer::~Renderer() {
	delete vertex_array_;
	delete render_grid_;
	delete texture_grid_;
	delete index_buffer_;
}

void jactorio::renderer::Renderer::draw(const glm::vec3 transform) const {
	// vertex_array_->bind();
	// index_buffer_->bind();
	//
	// const glm::mat4 model_matrix = translate(glm::mat4(1.f), transform);
	// setg_model_matrix(model_matrix);
	// update_shader_mvp();
	//
	// DEBUG_OPENGL_CALL(
	// 	glDrawElements(GL_TRIANGLES, index_buffer_->count(), GL_UNSIGNED_INT, nullptr)
	// ); // Pointer not needed as buffer is already bound
}

void jactorio::renderer::Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}
