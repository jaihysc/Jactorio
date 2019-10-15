#ifndef RENDERER_RENDERING_RENDERER_H
#define RENDERER_RENDERING_RENDERER_H

#include <glm/glm.hpp>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/rendering/mvp_manager.h"

namespace jactorio::renderer
{
	class Renderer
	{
		Vertex_array* vertex_array_{};
		Vertex_buffer *render_grid_{}, *texture_grid_{};
		Index_buffer* index_buffer_{};

		unsigned int grid_vertices_count_{};
		unsigned int grid_elements_count_{};

		static mvp_manager::Projection_tile_data tile_projection_matrix_;

	public:
		Renderer();
		~Renderer();

		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;

		static float tile_projection_matrix_offset;
		static void update_tile_projection_matrix();

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
	};
};

#endif // RENDERER_RENDERING_RENDERER_H
