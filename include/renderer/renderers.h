#ifndef RENDERERS_H
#define RENDERERS_H

#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_array.h"

namespace jactorio_renderer
{
	class Renderer
	{
	public:
		/*!
		 * Draws an object specified by va and ib, at position specified at translation
		 * Upper left corner is 0, 0
		 */
		void draw(const jactorio_renderer_gl::Vertex_array& va, const jactorio_renderer_gl::Index_buffer& ib, glm::vec3);
		void clear();
	};
	
};

#endif // RENDERERS_H
