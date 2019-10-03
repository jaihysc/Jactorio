#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/renderer.h"
#include "renderer/rendering/renderer_grid.h"

#include "renderer/manager/mvp_manager.h"
#include "renderer/manager/shader_manager.h"

#include "renderer/opengl/error.h"
#include "glm/detail/_noise.hpp"

jactorio_renderer_rendering::Renderer::Renderer() {
	// calculate_tile_properties must have been called first

	const unsigned int tile_count_x = jactorio_renderer::get_max_tile_count_x();
	const unsigned int tile_count_y = jactorio_renderer::get_max_tile_count_y();

	
	grid_vertices_count_ = (tile_count_x + 1) * (tile_count_y + 1);
	grid_elements_count_ = tile_count_x * tile_count_y;


	// #############################################
	// Initialization of vertex array and its buffers
	vertex_array_ = new jactorio_renderer_gl::Vertex_array();

	// Render grid
	render_grid_ = new jactorio_renderer_gl::Vertex_buffer(
		Renderer_grid::gen_render_tile_grid(
			tile_count_x,
			tile_count_y
		),
		grid_elements_count_ * 4 * 2 * sizeof(float)
	);
	vertex_array_->add_buffer(*render_grid_, 2, 0);
	
	
	// Spritemap positions
	texture_grid_ = new jactorio_renderer_gl::Vertex_buffer(
		Renderer_grid::gen_texture_grid(grid_elements_count_),
		grid_elements_count_ * 4 * 2 * sizeof(float)
	);
	vertex_array_->add_buffer(*texture_grid_, 2, 1);
	
	
	// Index buffer
	index_buffer_ = new jactorio_renderer_gl::Index_buffer(
		Renderer_grid::gen_render_grid_indices(
			tile_count_x,
			tile_count_y
		),
		grid_elements_count_ * 6
	);
}

jactorio_renderer_rendering::Renderer::~Renderer() {
	delete vertex_array_;
	delete render_grid_;
	delete texture_grid_;
	delete index_buffer_;
}

void jactorio_renderer_rendering::Renderer::draw(const glm::vec3 transform) const {
	vertex_array_->bind();
	index_buffer_->bind();

	const glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), transform);
	jactorio_renderer::setg_model_matrix(model_matrix);
	jactorio_renderer::update_shader_mvp();

	DEBUG_OPENGL_CALL(glDrawElements(GL_TRIANGLES, index_buffer_->count(), GL_UNSIGNED_INT, nullptr));  // Pointer not needed as buffer is already bound
}

void jactorio_renderer_rendering::Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}