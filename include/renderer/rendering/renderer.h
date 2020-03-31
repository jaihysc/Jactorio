// 
// renderer.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#pragma once

#include <glm/glm.hpp>

#include "renderer/rendering/renderer_layer.h"
#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::renderer
{
	class Renderer
	{
	public:
		Renderer();

	private:
		// #################################################
		// Sprites
		// Internal ids to spritemap positions
		static std::unordered_map<unsigned int, core::Quad_position> spritemap_coords_;

	public:
		static void set_spritemap_coords(
			const std::unordered_map<unsigned, core::Quad_position>& spritemap_coords);

		/**
		 * Use internal id of sprite prototype
		 */
		static core::Quad_position get_spritemap_coords(unsigned int internal_id);

	public:
		// #################################################
		// Rendering (Recalculated on window resize)

		/**
		 * 2 Rendering layers so that one can be drawn to while another is being rendered
		 * Since objects are of varying lengths, the layer must resize
		 */
		mutable Renderer_layer render_layer = Renderer_layer();
		mutable Renderer_layer render_layer2 = Renderer_layer();

		/**
		 * Using this to change zoom is discouraged <br>
		 * Use tile_width instead for better performance at high zoom levels
		 */
		float tile_projection_matrix_offset = 0;
		static unsigned short tile_width;

		/**
		 * Deletes and regenerates the opengl buffers / arrays used for rendering
		 * Also regenerates buffer used to update texture_grid_ Vertex_buffer
		 */
		void recalculate_buffers(unsigned short window_x, unsigned short window_y);

		/**
		 * Draws current data to the screen
		 * @param element_count Count of elements to draw (1 element = 6 indices)
		 */
		static void g_draw(unsigned int element_count);
		static void g_clear();


	private:
		// #################################################
		// Window properties
		static unsigned short window_width_;
		static unsigned short window_height_;

	public:
		J_NODISCARD static unsigned short get_window_width();
		J_NODISCARD static unsigned short get_window_height();


	private:
		// #################################################
		// Grid properties (rendering, MVP matrices)

		unsigned int grid_vertices_count_ = 0;
		unsigned int grid_elements_count_ = 0;

		unsigned short tile_count_x_ = 0;
		unsigned short tile_count_y_ = 0;

	public:
		J_NODISCARD unsigned short get_grid_size_x() const;
		J_NODISCARD unsigned short get_grid_size_y() const;

		/**
		* Updates projection matrix and zoom level
		*/
		void update_tile_projection_matrix();
	};
};

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
