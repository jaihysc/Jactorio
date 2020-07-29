// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_OVERLAY_ELEMENT_H
#define JACTORIO_INCLUDE_GAME_OVERLAY_ELEMENT_H
#pragma once

#include "core/coordinate_tuple.h"
#include "data/prototype/sprite.h"

namespace jactorio::game
{
	///
	/// \brief Named values for each z buffer number
	enum class OverlayLayer
	{
		// A separate layer is only needed when it needs to be accessed independently, otherwise join together in single layer
		general = 0,
		count_
	};

	constexpr int kOverlayLayerCount = static_cast<int>(OverlayLayer::count_);


	///
	/// \brief Holds items which do not adhere to the tiles for rendering
	class OverlayElement
	{
		/// z position of first layer
		static constexpr float kDefaultZPos = 0.4f;
		/// Every layer above increases its z position by this amount
		static constexpr float kZPosMultiplier = 0.01f;

	public:
		/// Offset from top left of chunk
		using OffsetT = float;

		/*
		OverlayElement(const data::Sprite& sprite,
		               const core::Position2<PositionT>& position,
		               const core::Position2<PositionT>& size)
			: OverlayElement(sprite, core::Position3<PositionT>{position, 0.f}, size) {
		}
		*/

		OverlayElement(const data::Sprite& sprite,
		               const core::Position2<OffsetT>& position,
		               const core::Position2<OffsetT>& size,
		               const OverlayLayer layer)
			: OverlayElement(sprite, core::Position3<OffsetT>{position, ToZPosition(layer)}, size) {
		}

		OverlayElement(const data::Sprite& sprite,
		               const core::Position3<OffsetT>& position,
		               const core::Position2<OffsetT>& size)
			: sprite(&sprite), position(position), size(size) {
		}

		// ======================================================================

		void SetZPosition(const OffsetT z_pos) {
			position.z = z_pos;
		}

		void SetZPosition(const OverlayLayer layer) {
			assert(layer != OverlayLayer::count_);
			SetZPosition(ToZPosition(layer));
		}

		J_NODISCARD static OffsetT ToZPosition(const OverlayLayer layer) noexcept {
			return kDefaultZPos + kZPosMultiplier * static_cast<float>(layer);
		}

		// ======================================================================

		const data::Sprite* sprite;

		data::Sprite::SetT spriteSet = 0;

		/// Distance (tiles) from top left of chunk to top left of sprite + z value
		core::Position3<OffsetT> position;

		/// Distance (tiles) the sprite spans
		core::Position2<OffsetT> size;
	};
}

#endif //JACTORIO_INCLUDE_GAME_OVERLAY_ELEMENT_H
