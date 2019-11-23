#ifndef RENDERER_RENDERING_RENDERER_H
#define RENDERER_RENDERING_RENDERER_H

#include <glm/glm.hpp>
#include <string>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::renderer
{
	class Renderer
	{
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
		
		// #################################################
		// Grid properties

		unsigned int grid_vertices_count_{};
		unsigned int grid_elements_count_{};


		unsigned short tile_count_x_{};
		unsigned short tile_count_y_{};

		static unsigned short window_width_;
		static unsigned short window_height_;


		// #################################################
		// Sprites

		// Internal names to spritemap positions
		std::unordered_map<std::string, Renderer_sprites::Image_position> spritemap_coords_;


	public:
		float tile_projection_matrix_offset{};
		unsigned short tile_width = 16;

		explicit Renderer(
			const std::unordered_map<std::string, Renderer_sprites::Image_position>& spritemap_coords);
		~Renderer();

		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;


		// #################################################
		// Rendering
	
		/**
		 * Draws current data to the screen
		 * @param transform Offset to shift the image drawn
		 */
		void draw(glm::vec3 transform) const;
		static void clear();

		[[nodiscard]] float* get_texture_grid_buffer() const {
			return texture_grid_buffer_;
		}
		
		void update_texture_grid_buffer() const {
			texture_grid_->set_buffer_data(texture_grid_buffer_, 0, 
			                               tile_count_x_ * tile_count_y_ * sizeof(float) * 8);
		}

		/**
		 * Deletes and regenerates the opengl buffers / arrays used for rendering
		 * Also regenerates buffer used to update texture_grid_ Vertex_buffer
		 */
		void recalculate_buffers(unsigned short window_x, unsigned short window_y);

		// #################################################
		// Grids (rendering, MVP matrices)
		
		[[nodiscard]] unsigned short get_grid_size_x() const {
			return tile_count_x_;
		}

		[[nodiscard]] unsigned short get_grid_size_y() const {
			return tile_count_y_;
		}

		/**
		* Updates projection matrix and zoom level
		*/
		void update_tile_projection_matrix();

		// #################################################
		// Sprites
		
		/**
		 * Sets sprite at specified grid index <br>
		 * 0, 0 is top left - grid starts from 0
		 * @param index_x
		 * @param index_y
		 * @param sprite_iname Internal name of the sprite
		 */
		void set_sprite(unsigned short index_x, unsigned short index_y, const std::string&
		                sprite_iname) const;

		[[nodiscard]] Renderer_sprites::Image_position get_sprite_spritemap_coords(
			const std::string& sprite_iname) const {
			return spritemap_coords_.at(sprite_iname);
		}
	};
};

#endif // RENDERER_RENDERING_RENDERER_H
