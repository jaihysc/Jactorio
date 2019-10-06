#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "renderer/opengl/vertex_buffer.h"

namespace jactorio
{
	namespace renderer
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
			
			void add_buffer(const Vertex_buffer& vb, unsigned span, unsigned location) const;

			void bind() const;
			static void unbind();
		};
	}
}

#endif // VERTEX_ARRAY_H
