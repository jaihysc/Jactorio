#ifndef RENDERER_OPENGL_INDEX_BUFFER_H
#define RENDERER_OPENGL_INDEX_BUFFER_H

namespace jactorio::renderer
{
	class Index_buffer
	{
		unsigned int id_{};
		unsigned int count_;

	public:
		// Count is numerical count
		Index_buffer(const unsigned int* data, unsigned int count);
		~Index_buffer();

		Index_buffer(const Index_buffer& other) = delete;
		Index_buffer(Index_buffer&& other) noexcept = delete;
		Index_buffer& operator=(const Index_buffer& other) = delete;
		Index_buffer& operator=(Index_buffer&& other) noexcept = delete;

		void bind() const;
		static void unbind();

		[[nodiscard]] unsigned int count() const;
	};
}

#endif // RENDERER_OPENGL_INDEX_BUFFER_H
