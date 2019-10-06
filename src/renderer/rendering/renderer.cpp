#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/renderer.h"
#include "renderer/rendering/renderer_grid.h"

#include "renderer/manager/mvp_manager.h"
#include "renderer/manager/shader_manager.h"

#include "renderer/opengl/error.h"


jactorio::renderer::Mvp_manager::Projection_tile_data
	jactorio::renderer::Renderer::tile_projection_matrix_ = Mvp_manager::Projection_tile_data{};
float jactorio::renderer::Renderer::tile_projection_matrix_offset = 0;

void jactorio::renderer::Renderer::update_tile_projection_matrix() {
	setg_projection_matrix(Mvp_manager::to_proj_matrix(tile_projection_matrix_, tile_projection_matrix_offset));
}

jactorio::renderer::Renderer::Renderer() {
	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	tile_projection_matrix_ = // 16 appears to be minimum tile width
		Mvp_manager::projection_calculate_tile_properties(16, m_viewport[2], m_viewport[3]);

	// jactorio::renderer::setg_projection_matrix(...) Must be called
	update_tile_projection_matrix();

	
	grid_vertices_count_ = (tile_projection_matrix_.tiles_x + 1) * (tile_projection_matrix_.tiles_y + 1);
	grid_elements_count_ = tile_projection_matrix_.tiles_x * tile_projection_matrix_.tiles_y;

	// #############################################
	// Initialization of vertex array and its buffers
	vertex_array_ = new Vertex_array();

	// Render grid
	render_grid_ = new Vertex_buffer(
		Renderer_grid::gen_render_tile_grid(
			tile_projection_matrix_.tiles_x,
			tile_projection_matrix_.tiles_y
		),
		grid_elements_count_ * 4 * 2 * sizeof(float)
	);
	vertex_array_->add_buffer(*render_grid_, 2, 0);
	
	
	// Spritemap positions
	texture_grid_ = new Vertex_buffer(
		Renderer_grid::gen_texture_grid(grid_elements_count_),
		grid_elements_count_ * 4 * 2 * sizeof(float)
	);
	vertex_array_->add_buffer(*texture_grid_, 2, 1);
	
	
	// Index buffer
	index_buffer_ = new Index_buffer(
		Renderer_grid::gen_render_grid_indices(
			tile_projection_matrix_.tiles_x,
			tile_projection_matrix_.tiles_y
		),
		grid_elements_count_ * 6
	);
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

	const glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), transform);
	setg_model_matrix(model_matrix);
	update_shader_mvp();

	DEBUG_OPENGL_CALL(
		glDrawElements(GL_TRIANGLES, index_buffer_->count(), GL_UNSIGNED_INT, nullptr)
	);  // Pointer not needed as buffer is already bound
}

void jactorio::renderer::Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}