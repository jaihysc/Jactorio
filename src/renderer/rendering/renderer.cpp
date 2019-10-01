#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/renderer.h"
#include "renderer/rendering/renderer_grid.h"

#include "renderer/manager/mvp_manager.h"
#include "renderer/manager/shader_manager.h"

#include "renderer/opengl/error.h"

// Temporary
float* jactorio_renderer_rendering::Renderer::gen_texture_grid() {
	const auto tex_coords = new float[grid_elements_count_ * 4 * 2];

	unsigned short index = 0;
	for (unsigned int i = 0; i < grid_elements_count_; ++i) {
		tex_coords[index++] = 0.f; tex_coords[index++] = 1.f,  // bottom left
		tex_coords[index++] = 1.f; tex_coords[index++] = 1.f;  // bottom right
		tex_coords[index++] = 1.f; tex_coords[index++] = 0.f;  // upper right
		tex_coords[index++] = 0.f; tex_coords[index++] = 0.f;  // upper left
	}

	return tex_coords;
}


jactorio_renderer_rendering::Renderer::Renderer()
{
	// calculate_tile_properties must have been called first
	
	grid_vertices_count_ = (jactorio_renderer::get_max_tile_count_x() + 1) * (jactorio_renderer::get_max_tile_count_y() + 1);
	grid_elements_count_ = jactorio_renderer::get_max_tile_count_x() * jactorio_renderer::get_max_tile_count_y();


	// #############################################
	// Initialization of vertex array and its buffers
	vertex_array_ = new jactorio_renderer_gl::Vertex_array();


	// Render grid
	render_grid_ = new jactorio_renderer_gl::Vertex_buffer(
		Renderer_grid::gen_render_grid(
			jactorio_renderer::get_max_tile_count_x() + 1,
			jactorio_renderer::get_max_tile_count_y() + 1
		),
		grid_vertices_count_ * 2 * sizeof(float)
	);
	vertex_array_->add_buffer(render_grid_, 2, 0);

	
	// Spritemap positions
	texture_grid_ = new jactorio_renderer_gl::Vertex_buffer(
		gen_texture_grid(),
		grid_elements_count_ * 4 * 2 * sizeof(float)
	);
	vertex_array_->add_buffer(texture_grid_, 2, 1);

	
	// Index buffer
}

jactorio_renderer_rendering::Renderer::~Renderer() {
	delete render_grid_, texture_grid_;
}

void jactorio_renderer_rendering::Renderer::draw(const jactorio_renderer_gl::Vertex_array& va, const jactorio_renderer_gl::Index_buffer& ib, const glm::vec3 transform) {
	va.bind();
	ib.bind();

	const glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), transform);
	jactorio_renderer::setg_model_matrix(model_matrix);
	jactorio_renderer::update_shader_mvp();

	DEBUG_OPENGL_CALL(glDrawElements(GL_TRIANGLES, ib.count(), GL_UNSIGNED_INT, nullptr));  // Pointer not needed as buffer is already bound
}

void jactorio_renderer_rendering::Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}