#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

namespace jactorio
{
	namespace renderer
	{
		class Vertex_buffer
		{
			unsigned int id_{};
		public:
			// Size in bytes
			Vertex_buffer(const void* data, unsigned int size);
			~Vertex_buffer();

			Vertex_buffer(const Vertex_buffer& other) = delete;
			Vertex_buffer(Vertex_buffer&& other) noexcept = delete;
			Vertex_buffer& operator=(const Vertex_buffer& other) = delete;
			Vertex_buffer& operator=(Vertex_buffer&& other) noexcept = delete;
			
			void bind() const;
			static void unbind();
		};	
	}
}

#endif // VERTEX_BUFFER_H
