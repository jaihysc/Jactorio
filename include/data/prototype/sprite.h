// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/24/2019

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
#pragma once

#include <string>
#include <vector>

#include "jactorio.h"
#include "core/data_type.h"
#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	///
	/// Unique data: Renderable_data
	class Sprite final : public PrototypeBase
	{
	public:
		PROTOTYPE_CATEGORY(sprite);

		enum class SpriteGroup
		{
			terrain = 0,
			gui,
			count_
		};

		Sprite() = default;
		explicit Sprite(const std::string& sprite_path);
		Sprite(const std::string& sprite_path, std::vector<SpriteGroup> group);

		~Sprite() override;

		Sprite(const Sprite& other);
		Sprite(Sprite&& other) noexcept;

		Sprite& operator=(Sprite other) {
			swap(*this, other);
			return *this;
		}

		friend void swap(Sprite& lhs, Sprite& rhs) noexcept {
			using std::swap;
			swap(static_cast<PrototypeBase&>(lhs), static_cast<PrototypeBase&>(rhs));
			swap(lhs.group, rhs.group);
			swap(lhs.frames, rhs.frames);
			swap(lhs.sets, rhs.sets);
			swap(lhs.trim, rhs.trim);
			swap(lhs.width_, rhs.width_);
			swap(lhs.height_, rhs.height_);
			swap(lhs.bytesPerPixel_, rhs.bytesPerPixel_);
			swap(lhs.spritePath_, rhs.spritePath_);
			swap(lhs.spriteBuffer_, rhs.spriteBuffer_);
		}

		// ======================================================================
		// Sprite (image) properties

		using SetT = uint16_t;
		using FrameT = uint16_t;
		using TrimT = uint16_t;

		///
		/// \brief Group(s) determines which spritemap(s) this sprite is placed on
		PYTHON_PROP_REF(Sprite, std::vector<SpriteGroup>, group);

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
		/// \brief If true : X = Set, Y = Frame,
		///			  false: Y = Set, X = Frame
		PYTHON_PROP_REF_I(Sprite, bool, invertSetFrame, false);

		///
		/// \brief Animation frames, X axis, indexed by 0 based index, 1 if single
		PYTHON_PROP_REF_I(Sprite, FrameT, frames, 1);
		///
		/// \brief Y axis, indexed by 0 based index, 1 if single
		PYTHON_PROP_REF_I(Sprite, SetT, sets, 1);


		///
		/// \brief Pixels to remove from the border when get_coords() is called
		PYTHON_PROP_REF_I(Sprite, TrimT, trim, 0);


		///
		/// \return true is Sprite is in specified group
		bool IsInGroup(SpriteGroup group);

		///
		/// \brief If group is empty, it is set to the group provided
		void DefaultSpriteGroup(const std::vector<SpriteGroup>& new_group);

	private:
		// Image properties
		int width_ = 0, height_ = 0, bytesPerPixel_ = 0;

		// Path is already resolved
		std::string spritePath_;
		unsigned char* spriteBuffer_ = nullptr;

		///
		/// \brief Loads image from file
		/// load_image only sets the sprite_path and calls this
		/// \exception DataException Failed to load from file
		void LoadImageFromFile();


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
		/// \brief Performs the following adjustments to set and frame
		/// \tparam InvertSet set is flipped horizontally if true
		/// \param set Modulus of total number of sets
		/// \param frame Modulus of total number of frames, every multiple of frames increases set by 1
		template <bool InvertSet>
		void AdjustSetFrame(SetT& set, FrameT& frame) const {
			set %= sets;
			set += frame / frames;
			frame = frame % frames;

			if constexpr (InvertSet) {
				// Single opengl flips images horizontally, select sets mirrored
				set = sets - set - 1;
			}
		}

	public:
		///
		/// \brief Gets OpenGl UV coordinates for region within a sprite, applying a deduction of trim pixels around the border
		/// \remark Requires width_ and height_ to be initialized
		/// \return UV coordinates for set, frame within sprite (0, 0) is top left
		J_NODISCARD core::QuadPosition GetCoords(SetT set, FrameT frame) const;

		// ======================================================================
		// Sprite ptr

		J_NODISCARD const unsigned char* GetSpritePtr() const;

		///
		/// \brief Gets size of image on X axis
		J_NODISCARD unsigned int GetWidth() const { return width_; }

		///
		/// \brief Gets size of image on Y axis
		J_NODISCARD unsigned int GetHeight() const { return height_; }


		///
		/// \brief Loads a sprite from sprite_path into member sprite
		/// \remark Do not include ~/data/
		Sprite* LoadImage(const std::string& image_path);


		void PostLoadValidate() const override;


		// ======================================================================
#ifdef JACTORIO_BUILD_TEST
		void SetHeight(const int height) { height_ = height; }
		void SetWidth(const int width) { width_ = width; }
#endif
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_SPRITE_H
