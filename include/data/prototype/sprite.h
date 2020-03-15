// 
// sprite.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 11/24/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
#pragma once

#include "jactorio.h"

#include <string>
#include <vector>

#include "prototype_base.h"

namespace jactorio::data
{
	class Sprite final : public Prototype_base
	{
	public:
		enum class sprite_group
		{
			terrain = 0,
			gui,
			_count
		};

		/**
		 * Group(s) determines which spritemap(s) this sprite is placed on
		 */
		PYTHON_PROP_REF(Sprite, std::vector<sprite_group>, group);

		/**
		 * @return true is Sprite is in specified group
		 */
		bool is_in_group(sprite_group group);

	private:
		// Image properties
		int width_, height_, bytes_per_pixel_;

		// Path is already resolved
		std::string sprite_path_;
		unsigned char* sprite_buffer_;

		/**
		 * Actually loads the image, load_image only sets the sprite_path and calls this
		 * @exception Data_exception Failed to load from file
		 */
		void load_image_from_file();

	public:
		Sprite();
		explicit Sprite(const std::string& sprite_path);
		Sprite(const std::string& sprite_path, std::vector<sprite_group> group);

		~Sprite() override;


		Sprite(const Sprite& other);
		Sprite(Sprite&& other) noexcept = default;

		Sprite& operator=(const Sprite& other);
		Sprite& operator=(Sprite&& other) noexcept = default;

		// =======================================
		// Sprite ptr

		J_NODISCARD const unsigned char* get_sprite_data_ptr() const;

		/**
		 * Provided sprite_data pointer will be managed by the Sprite class, it must not be deleted
		 */
		void set_sprite_data_ptr(unsigned char* sprite_data, unsigned sprite_width, unsigned sprite_height);


		/**
		 * Gets size of image on X axis
		 */
		J_NODISCARD unsigned int get_width() const;

		/**
		 * Gets size of image on Y axis
		 */
		J_NODISCARD unsigned int get_height() const;


		/**
		 * Loads a sprite from sprite_path into member sprite <br>
		 * Do not include ~/data/
		 */
		Sprite* load_image(const std::string& image_path);


		void post_load_validate() const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
