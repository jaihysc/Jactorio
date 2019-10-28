#ifndef RENDERER_OPENGL_TEXTURE_H
#define RENDERER_OPENGL_TEXTURE_H

#include <SFML/Graphics/Image.hpp>

namespace jactorio::renderer
{
	class Texture
	{
		unsigned int renderer_id_;
		int width_, height_;

	public:
		explicit Texture(const sf::Image& image);
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
