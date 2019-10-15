#ifndef RENDERER_OPENGL_TEXTURE_H
#define RENDERER_OPENGL_TEXTURE_H

#include <string>

namespace jactorio::renderer
{
	class Texture
	{
		unsigned int renderer_id_;
		int width_, height_, bytes_per_pixel_;

		std::string texture_filepath_;
		unsigned char* texture_buffer_;

	public:
		explicit Texture(const std::string& internal_name);
		~Texture();

		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

		void bind(unsigned int slot = 0) const;
		void unbind() const;

		[[nodiscard]] int get_width() const { return width_; }
		[[nodiscard]] int get_height() const { return height_; }
	};
}

#endif // RENDERER_OPENGL_TEXTURE_H
