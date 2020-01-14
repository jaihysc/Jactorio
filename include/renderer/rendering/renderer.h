#ifndef RENDERER_RENDERING_RENDERER_H
#define RENDERER_RENDERING_RENDERER_H

#include <glm/glm.hpp>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/rendering/renderer_layer.h"
#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::renderer
{
	class Renderer
	{		
	public:
		explicit Renderer();
		~Renderer();

		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;
		
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
		
	private:
		// #################################################
		// Rendering (Recalculated on window resize)

		Vertex_array* vertex_array_ = nullptr;

		/**
		 * Deletes all heap allocated data
		 */
		void delete_data() const;
	public:
		Renderer_layer* render_layer = nullptr;
		
		/**
		 * Using this to change zoom is discouraged <br>
		 * Use tile_width instead for better performance at high zoom levels
		 */
		float tile_projection_matrix_offset{};
		static unsigned short tile_width;

		/**
		 * Deletes and regenerates the opengl buffers / arrays used for rendering
		 * Also regenerates buffer used to update texture_grid_ Vertex_buffer
		 */
		void recalculate_buffers(unsigned short window_x, unsigned short window_y);

		/**
		 * Draws current data to the screen
		 * @param transform Offset to shift the image drawn
		 */
		void g_draw(glm::vec3 transform) const;
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

		unsigned int grid_vertices_count_{};
		unsigned int grid_elements_count_{};

		unsigned short tile_count_x_{};
		unsigned short tile_count_y_{};
		
	public:
		J_NODISCARD unsigned short get_grid_size_x() const;
		J_NODISCARD unsigned short get_grid_size_y() const;

		/**
		* Updates projection matrix and zoom level
		*/
		void update_tile_projection_matrix();
	};
};

#endif // RENDERER_RENDERING_RENDERER_H
