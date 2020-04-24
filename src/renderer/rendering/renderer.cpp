// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <GL/glew.h>

#include "renderer/rendering/renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/opengl/error.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/mvp_manager.h"

unsigned short jactorio::renderer::Renderer::window_width_ = 0;
unsigned short jactorio::renderer::Renderer::window_height_ = 0;

unsigned short jactorio::renderer::Renderer::tile_width = 6;

std::unordered_map<unsigned int, jactorio::core::Quad_position> jactorio::renderer::Renderer::spritemap_coords_{};

void jactorio::renderer::Renderer::set_spritemap_coords(
	const std::unordered_map<unsigned, core::Quad_position>& spritemap_coords) {
	spritemap_coords_ = spritemap_coords;
}

jactorio::core::Quad_position jactorio::renderer::Renderer::get_spritemap_coords(const unsigned internal_id) {
	return spritemap_coords_.at(internal_id);
}


// non static

jactorio::renderer::Renderer::Renderer() {
	// Initialize model matrix
	const glm::mat4 model_matrix = translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
	setg_model_matrix(model_matrix);
	update_shader_mvp();

	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	render_layer.g_init_buffer();
	render_layer2.g_init_buffer();

	recalculate_buffers(m_viewport[2], m_viewport[3]);
}

void jactorio::renderer::Renderer::recalculate_buffers(const unsigned short window_x,
                                                       const unsigned short window_y) {
	// Initialize fields
	window_width_ = window_x;
	window_height_ = window_y;
	update_tile_projection_matrix();

	// Raise the bottom and right by tile_width so the last tile has enough space to render out
	tile_count_x_ = window_width_ / tile_width + 1;
	tile_count_y_ = window_height_ / tile_width + 1;

	grid_elements_count_ = tile_count_x_ * tile_count_y_;


	// Render layer (More may be reserved as needed by the renderer)
	render_layer.reserve(grid_elements_count_);
	render_layer2.reserve(grid_elements_count_);
}


// openGL methods

void jactorio::renderer::Renderer::g_draw(const unsigned int element_count) {
	DEBUG_OPENGL_CALL(
		// There are 6 indices for each tile
		glDrawElements(GL_TRIANGLES, element_count * 6, GL_UNSIGNED_INT, nullptr)
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

	setg_projection_matrix(to_proj_matrix(window_width_, window_height_, tile_projection_matrix_offset));
}
