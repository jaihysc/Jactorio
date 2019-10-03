#ifndef RENDERING_RENDERER_RENDERER_H
#define RENDERING_RENDERER_RENDERER_H

#include <glm/glm.hpp>

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/vertex_array.h"

namespace jactorio_renderer_rendering
{
	class Renderer
	{
		jactorio_renderer_gl::Vertex_array* vertex_array_;
		jactorio_renderer_gl::Vertex_buffer* render_grid_, *texture_grid_;
		jactorio_renderer_gl::Index_buffer* index_buffer_;
		
		unsigned int grid_vertices_count_;
		unsigned int grid_elements_count_;


	public:
		Renderer();
		~Renderer();

		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;

		/*!
		 * Draws an object specified by va and ib, at position specified at translation
		 * Upper left corner is 0, 0
		 */
		void draw(glm::vec3 transform) const;
		void clear();
	};
	
};

#endif // RENDERING_RENDERER_RENDERER_H