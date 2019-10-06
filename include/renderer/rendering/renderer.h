#ifndef RENDERING_RENDERER_RENDERER_H
#define RENDERING_RENDERER_RENDERER_H

#include <glm/glm.hpp>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/manager/mvp_manager.h"

namespace jactorio
{
	namespace renderer
	{
		class Renderer
		{
			Vertex_array* vertex_array_;
			Vertex_buffer* render_grid_, *texture_grid_;
			Index_buffer* index_buffer_;
			
			unsigned int grid_vertices_count_;
			unsigned int grid_elements_count_;

			static Mvp_manager::Projection_tile_data tile_projection_matrix_;


		public:
			Renderer();
			~Renderer();

			Renderer(const Renderer& other) = delete;
			Renderer(Renderer&& other) = delete;

			Renderer& operator=(const Renderer& other) = delete;
			Renderer& operator=(Renderer&& other) = delete;

			static float tile_projection_matrix_offset;
			static void update_tile_projection_matrix();
			
			/*!
			 * Draws an object specified by va and ib, at position specified at translation
			 * Upper left corner is 0, 0
			 */
			void draw(glm::vec3 transform) const;
			void clear();
		};
	}

};

#endif // RENDERING_RENDERER_RENDERER_H