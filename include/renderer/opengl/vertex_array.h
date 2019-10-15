#ifndef RENDERER_OPENGL_VERTEX_ARRAY_H
#define RENDERER_OPENGL_VERTEX_ARRAY_H

#include "renderer/opengl/vertex_buffer.h"

namespace jactorio::renderer
{
	class Vertex_array
	{
		unsigned int id_{};

	public:
		Vertex_array();
		~Vertex_array();

		Vertex_array(const Vertex_array& other) = delete;
		Vertex_array(Vertex_array&& other) noexcept = delete;
		Vertex_array& operator=(const Vertex_array& other) = delete;
		Vertex_array& operator=(Vertex_array&& other) noexcept = delete;

		/**
		 * Adds specified buffer to the vertex array <br>
		 * Vertex_buffer must be deleted manually, it is not managed by the vertex array
		 * @param vb Vertex buffer to add to vertex array
		 * @param span Size in bytes of one set of coordinates
		 * @param location Slot in vertex array in which vertex buffer is placed <br>
		 * This must be managed manually to avoid conflicts
		 */
		void add_buffer(const Vertex_buffer& vb, unsigned span,
		                unsigned location) const;

		void bind() const;
		static void unbind();
	};
}

#endif // RENDERER_OPENGL_VERTEX_ARRAY_H
