// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#pragma once

#include <cmath>

#include "core/data_type.h"
#include "data/prototype/sprite.h"
#include "data/prototype/type.h"
#include "game/world/world_data.h"

namespace jactorio
{
	namespace data
	{
		class PrototypeManager;
	}

	namespace game
	{
		class PlayerData;
		class ChunkTileLayer;
		class Chunk;
	}

	namespace renderer
	{
		class RendererLayer;
	}
}

namespace jactorio::data
{
	///
	/// \brief Inherit to allow drawing portions of a sprite
	struct IRenderableData
	{
	protected:
		IRenderableData()          = default;
		virtual ~IRenderableData() = default;

		explicit IRenderableData(const Sprite::SetT set)
			: set(set) {
		}

	public:
		Sprite::SetT set = 0;
	};

	///
	/// \brief For inheriting by IPrototypeRenderable, thus enabling usage by renderer
	class IRenderable
	{
	protected:
		IRenderable()          = default;
		virtual ~IRenderable() = default;

		IRenderable(const IRenderable& other)                = default;
		IRenderable(IRenderable&& other) noexcept            = default;
		IRenderable& operator=(const IRenderable& other)     = default;
		IRenderable& operator=(IRenderable&& other) noexcept = default;

	public:
		///
		/// \brief Gets a sprite corresponding to the provided set 
		J_NODISCARD virtual Sprite* OnRGetSprite(Sprite::SetT set) const = 0;

		///
		/// \brief Maps a placementOrientation to a <set, frame>
		J_NODISCARD virtual Sprite::SetT OnRGetSpriteSet(Orientation orientation,
		                                                 game::WorldData& world_data,
		                                                 const WorldCoord& world_coords) const = 0;

		///
		/// \brief Gets frame for sprite corresponding to provided game tick 
		J_NODISCARD virtual Sprite::FrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data,
		                                                     GameTickT game_tick) const = 0;

		///
		/// \brief Displays the menu associated with itself with the provided data
		virtual bool OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
		                        game::ChunkTileLayer* tile_layer) const = 0;

		///
		/// \param pixel_offset Pixels to top left of current tile
		virtual void OnRDrawUniqueData(renderer::RendererLayer& layer, const SpriteUvCoordsT& uv_coords,
		                               const core::Position2<float>& pixel_offset,
		                               const UniqueDataBase* unique_data) const {
		}

	protected:
		// ======================================================================
		// Methods for OnRGetSpriteFrame

		using AnimationSpeed = double;

		///
		/// \brief Every set / frame of a sprite is part of the same animation
		static Sprite::FrameT AllOfSprite(Sprite& sprite,
		                                  const GameTickT game_tick,
		                                  const AnimationSpeed speed = 1) {
			assert(speed > 0);

			return static_cast<GameTickT>(speed * game_tick) % (static_cast<uint64_t>(sprite.frames) * sprite.sets);
		}

		///
		/// \brief Every set / frame of a sprite is part of the same animation, plays forwards then backwards
		static Sprite::FrameT AllOfSpriteReversing(Sprite& sprite,
		                                           const GameTickT game_tick,
		                                           const AnimationSpeed speed = 1) {
			assert(speed > 0);

			// s = speed, f = frames
			// y = abs( mod(s * (x - 1/s * (f - 1)), (f * 2 - 2)) - f + 1 )

			// Graph this function to make it easier to understand

			const auto frames = static_cast<uint16_t>(sprite.frames) * sprite.sets;

			// Shift the peak (which is at x = 0) such that when x = 0, y = 0
			const auto adjusted_x = game_tick - (1.f / speed) * (frames - 1);

			const auto v_l = static_cast<int64_t>(speed * abs(adjusted_x));
			const auto v_r = static_cast<int64_t>(frames) * 2 - 2;

			const auto val = (v_l % v_r) - frames + 1;
			assert(val < frames);

			return abs(val);
		}

		///
		/// \brief Every frame of a set
		static Sprite::FrameT AllOfSet(Sprite& sprite,
		                               const GameTickT game_tick,
		                               const AnimationSpeed speed = 1) {
			assert(speed > 0);
			return static_cast<GameTickT>(speed * game_tick) % sprite.frames;
		}
	};
}

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
