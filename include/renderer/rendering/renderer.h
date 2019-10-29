#ifndef RENDERER_RENDERING_RENDERER_H
#define RENDERER_RENDERING_RENDERER_H

#include <glm/glm.hpp>
#include <string>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/rendering/renderer_sprites.h"

namespace jactorio::renderer
{
	class Renderer
	{
		Vertex_array* vertex_array_{};
		Vertex_buffer *render_grid_{}, *texture_grid_{};
		Index_buffer* index_buffer_{};

		unsigned int grid_vertices_count_{};
		unsigned int grid_elements_count_{};


		unsigned short tile_count_x_;
		unsigned short tile_count_y_;


		// Internal names to spritemap positions
		std::unordered_map<std::string, Renderer_sprites::Image_position> spritemap_coords_;
		
		static unsigned short window_width_;
		static unsigned short window_height_;

	public:
		explicit Renderer(
			const std::unordered_map<std::string, Renderer_sprites::Image_position>& spritemap_coords);
		~Renderer();

		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;

		static float tile_projection_matrix_offset;

		/**
		 * Draws current data to the screen
		 * @param transform Offset to shift the image drawn
		 */
		void draw(glm::vec3 transform) const;
		void clear();

		/**
		 * Deletes and regenerates the opengl buffers / arrays used for rendering
		 */
		void recalculate_buffers(unsigned short window_x, unsigned short window_y);

		/**
		 * Sets sprite at specified grid index <br>
		 * 0, 0 is top left - grid starts from 0
		 * @param index_x
		 * @param index_y
		 * @param sprite_iname Internal name of the sprite
		 */
		void set_sprite(unsigned short index_x, unsigned short index_y, const std::string&
		                sprite_iname);

		
		[[nodiscard]] unsigned short get_grid_size_x() const {
			return tile_count_x_;
		}

		[[nodiscard]] unsigned short get_grid_size_y() const {
			return tile_count_y_;
		}
		

		/**
		 * Updates projection matrix and zoom level
		 */
		static void update_tile_projection_matrix();
	};
};

#endif // RENDERER_RENDERING_RENDERER_H
