#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

namespace jactorio_renderer_gl
{
	class Vertex_buffer
	{
	private:
		unsigned int id_{};
	public:
		// Size in bytes
		Vertex_buffer(const void* data, unsigned int size);
		~Vertex_buffer();

		void bind() const;
		static void unbind();
	};
}

#endif // VERTEX_BUFFER_H
