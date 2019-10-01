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
		
		unsigned int grid_vertices_count_;
		unsigned int grid_elements_count_;


		float* gen_texture_grid();

		
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
		void draw(const jactorio_renderer_gl::Vertex_array& va, const jactorio_renderer_gl::Index_buffer& ib, glm::vec3 transform);
		void clear();
	};
	
};

#endif // RENDERING_RENDERER_RENDERER_H