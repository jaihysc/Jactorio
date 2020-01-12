#ifndef RENDERER_OPENGL_VERTEX_BUFFER_H
#define RENDERER_OPENGL_VERTEX_BUFFER_H

namespace jactorio::renderer
{
	class Vertex_buffer
	{
		unsigned int id_{};
	public:
		/**
		 * @param data
		 * @param size Size of the data in bytes
		 */
		Vertex_buffer(const void* data, unsigned int size);
		~Vertex_buffer();

		
		Vertex_buffer(const Vertex_buffer& other) = delete;
		Vertex_buffer(Vertex_buffer&& other) noexcept = delete;
		Vertex_buffer& operator=(const Vertex_buffer& other) = delete;
		Vertex_buffer& operator=(Vertex_buffer&& other) noexcept = delete;

		void set_buffer_data(const void* data, const unsigned offset, const unsigned size) const;
		
		void bind() const;
		static void unbind();
	};
}

#endif // RENDERER_OPENGL_VERTEX_BUFFER_H
