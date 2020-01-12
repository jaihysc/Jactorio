#ifndef RENDERER_RENDERING_RENDERER_H
#define RENDERER_RENDERING_RENDERER_H

#include <glm/glm.hpp>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/vertex_array.h"
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
		// Rendering
		
		// Buffer of spritemap positions for updating the texture_grid_ Vertex_buffer
		// Recalculated on window resize
		float* texture_grid_buffer_ = nullptr;
		
		Vertex_array* vertex_array_ = nullptr;
		Vertex_buffer *render_grid_ = nullptr, *texture_grid_ = nullptr;
		Index_buffer* index_buffer_ = nullptr;

		/**
		 * Deletes all heap allocated data
		 */
		void delete_data() const;
	public:
		/**
		 * Using this to change zoom is discouraged <br>
		 * Use tile_width instead for better performance at high zoom levels
		 */
		float tile_projection_matrix_offset{};
		static unsigned short tile_width;

		/**
		 * Draws current data to the screen
		 * @param transform Offset to shift the image drawn
		 */
		void draw(glm::vec3 transform) const;
		static void clear();

		// Grid for UV coordinates
		[[nodiscard]] float* get_texture_grid_buffer() const;
		void update_texture_grid_buffer() const;

		/**
		 * Deletes and regenerates the opengl buffers / arrays used for rendering
		 * Also regenerates buffer used to update texture_grid_ Vertex_buffer
		 */
		void recalculate_buffers(unsigned short window_x, unsigned short window_y);

	private:
		// #################################################
		// Window properties
		static unsigned short window_width_;
		static unsigned short window_height_;
	public:
		[[nodiscard]] static unsigned short get_window_width();
		[[nodiscard]] static unsigned short get_window_height();

	private:
		// #################################################
		// Grid properties (rendering, MVP matrices)

		unsigned int grid_vertices_count_{};
		unsigned int grid_elements_count_{};

		unsigned short tile_count_x_{};
		unsigned short tile_count_y_{};
	public:
		[[nodiscard]] unsigned short get_grid_size_x() const;
		[[nodiscard]] unsigned short get_grid_size_y() const;

		/**
		* Updates projection matrix and zoom level
		*/
		void update_tile_projection_matrix();
	};
};

#endif // RENDERER_RENDERING_RENDERER_H
