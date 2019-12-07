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
		// #################################################
		// Sprites
		// Internal ids to spritemap positions
		static std::unordered_map<unsigned int, Renderer_sprites::Image_position> spritemap_coords_;
	public:
		static void set_spritemap_coords(
			const std::unordered_map<unsigned, Renderer_sprites::Image_position>& spritemap_coords);

		/**
		 * Use internal id of sprite prototype
		 */
		static Renderer_sprites::Image_position get_spritemap_coords(unsigned int internal_id);
		
	private:
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


	public:
		/**
		 * Using this to change zoom is discouraged <br>
		 * Use tile_width instead for better performance at high zoom levels
		 */
		float tile_projection_matrix_offset{};
		static unsigned short tile_width;

		explicit Renderer();
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

		/*// #################################################
		// Sprites
		
		/**
		 * Sets sprite at specified grid index <br>
		 * 0, 0 is top left - grid starts from 0
		 * @param index_x
		 * @param index_y
		 * @param sprite_iname Internal name of the sprite
		 #1#
		void set_sprite(unsigned short index_x, unsigned short index_y, const std::string&
		                sprite_iname) const;*/
	};
};

#endif // RENDERER_RENDERING_RENDERER_H
