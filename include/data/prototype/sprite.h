#ifndef DATA_PROTOTYPE_SPRITE_H
#define DATA_PROTOTYPE_SPRITE_H

#include <string>

#include "prototype_base.h"

namespace jactorio::data
{
	class Sprite : public Prototype_base
	{
	public:
		enum class sprite_group
		{
			none,
			terrain,
			gui
		};

		/**
		 * Group determines which spritemap this is placed on
		 */
		PYTHON_PROP_REF(Sprite, sprite_group, group)
		
	private:
		// Image properties
		int width_, height_, bytes_per_pixel_;

		// Path is already resolved
		std::string sprite_path_;
		unsigned char* sprite_buffer_;
		
		/**
		 * Actually loads the image, load_image only sets the sprite_path and calls this
		 */
		void load_image_from_file();
		
	public:
		Sprite();
		explicit Sprite(const std::string& sprite_path);
		Sprite(const std::string& sprite_path, sprite_group group);

		~Sprite() override;


		Sprite(const Sprite& other);

		Sprite(Sprite&& other) noexcept = delete;

		Sprite& operator=(const Sprite& other);

		Sprite& operator=(Sprite&& other) noexcept = delete;

	public:
		
		[[nodiscard]] const unsigned char* get_sprite_data_ptr() const;

		/**
		 * Provided sprite_data pointer will be managed by the Sprite class, it must not be deleted
		 */
		void set_sprite_data_ptr(unsigned char* sprite_data, unsigned sprite_width, unsigned sprite_height);

		
		/**
		 * Gets size of image on X axis
		 */
		[[nodiscard]] unsigned int get_width() const;

		/**
		 * Gets size of image on Y axis
		 */
		[[nodiscard]] unsigned int get_height() const;

		
		/**
		 * Loads a sprite from sprite_path into member sprite <br>
		 * Do not include ~/data/
		 */
		Sprite* load_image(const std::string& image_path);
	};
}

#endif // DATA_PROTOTYPE_SPRITE_H
