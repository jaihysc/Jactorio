// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/24/2019

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
#pragma once

#include "jactorio.h"

#include <string>
#include <vector>


#include "core/data_type.h"
#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	///
	/// Unique data: Renderable_data
	class Sprite final : public Prototype_base
	{
	public:
		enum class spriteGroup
		{
			terrain = 0,
			gui,
			count_
		};

		///
		/// \brief Group(s) determines which spritemap(s) this sprite is placed on
		PYTHON_PROP_REF(Sprite, std::vector<spriteGroup>, group);

		/*
		 *     F0 F1 F2 F3 F4
		 *    ----------------
		 * S0 |  |  |  |  |  |
		 *    ----------------
		 * S1 |  |  |  |  |  |
		 *    ----------------
		 * S2 |  |  |  |  |  |
		 *    ----------------
		 */

		///
		/// \brief Animation frames, X axis, indexed by 0 based index, 1 if single
		PYTHON_PROP_REF_I(Sprite, uint16_t, frames, 1);
		///
		/// \brief Y axis, indexed by 0 based index, 1 if single
		PYTHON_PROP_REF_I(Sprite, uint16_t, sets, 1);

		///
		/// \brief Pixels to remove from the border when get_coords() is called
		PYTHON_PROP_REF_I(Sprite, uint16_t, trim, 0);


		///
		/// \return true is Sprite is in specified group
		bool is_in_group(spriteGroup group);

	private:
		// Image properties
		int width_, height_, bytes_per_pixel_;

		// Path is already resolved
		std::string sprite_path_;
		unsigned char* sprite_buffer_;

		///
		/// \brief Loads image from file
		/// load_image only sets the sprite_path and calls this
		/// \exception Data_exception Failed to load from file
		void load_image_from_file();

	public:
		PROTOTYPE_CATEGORY(sprite);

		Sprite();
		explicit Sprite(const std::string& sprite_path);
		Sprite(const std::string& sprite_path, std::vector<spriteGroup> group);

		~Sprite() override;


		Sprite(const Sprite& other);
		Sprite(Sprite&& other) noexcept;

		Sprite& operator=(Sprite other) {
			swap(*this, other);
			return *this;
		}

		friend void swap(Sprite& lhs, Sprite& rhs) noexcept {
			using std::swap;
			swap(static_cast<Prototype_base&>(lhs), static_cast<Prototype_base&>(rhs));
			swap(lhs.group, rhs.group);
			swap(lhs.frames, rhs.frames);
			swap(lhs.sets, rhs.sets);
			swap(lhs.trim, rhs.trim);
			swap(lhs.width_, rhs.width_);
			swap(lhs.height_, rhs.height_);
			swap(lhs.bytes_per_pixel_, rhs.bytes_per_pixel_);
			swap(lhs.sprite_path_, rhs.sprite_path_);
			swap(lhs.sprite_buffer_, rhs.sprite_buffer_);
		}

		// ======================================================================
		// Image extraction

		/*
		 * Actual set used in method is 'input set modulus by total sets',
		 * allowing for sets for different sprites to be referenced

		 * e.g:
		 * With 4 different sprites, total of 10 sets per sprite
		 * The following set ranges will correspond to the 4 sprites:
		 * 0  -  9: Sprite 1
		 * 10 - 19: Sprite 2
		 * 20 - 29: Sprite 3
		 * 30 - 39: Sprite 4
		 */

		///
		/// \param mset Will be modulus by total sets
		/// \param frame 
		/// \return UV coordinates for set, frame within sprite (0, 0) is top left
		J_NODISCARD core::Quad_position get_coords(uint16_t mset, uint16_t frame) const;

		///
		/// \brief Same as get_coords, but applies a deduction of trim pixels around the border
		/// \remark Requires width_ and height_ to be initialized
		/// \param mset Will be modulus by total sets
		/// \param frame 
		/// \return UV coordinates for set, frame within sprite (0, 0) is top left
		J_NODISCARD core::Quad_position get_coords_trimmed(uint16_t mset, uint16_t frame) const;

		// ======================================================================
		// Sprite ptr

		J_NODISCARD const unsigned char* get_sprite_data_ptr() const;

		///
		/// \brief Gets size of image on X axis
		J_NODISCARD unsigned int get_width() const { return width_; }

		///
		/// \brief Gets size of image on Y axis
		J_NODISCARD unsigned int get_height() const { return height_; }


		///
		/// \brief Loads a sprite from sprite_path into member sprite
		/// \remark Do not include ~/data/
		Sprite* load_image(const std::string& image_path);


		void post_load_validate() const override;


		// ======================================================================
		// Test only
		J_TEST_USE_ONLY void set_height(const int height) { this->height_ = height; }
		J_TEST_USE_ONLY void set_width(const int width) { this->width_ = width; }
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
