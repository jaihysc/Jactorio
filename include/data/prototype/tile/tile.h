// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef DATA_PROTOTYPE_TILE_TILE_H
#define DATA_PROTOTYPE_TILE_TILE_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/interface/renderable.h"

namespace jactorio::data
{
	///
	/// \brief Tiles which are auto-placed by the world generator <br>
	/// Configure how the world generator places these tiles with a Noise_layer
	class Tile final : public PrototypeBase, public IRenderable
	{
	public:
		PROTOTYPE_CATEGORY(tile);

		Tile() = default;

		///
		/// \param sprite_ptr Points to a sprite prototype
		explicit Tile(Sprite* sprite_ptr)
			: sprite(sprite_ptr) {
		}

		~Tile() override = default;

		Tile(const Tile& other)                = default;
		Tile(Tile&& other) noexcept            = default;
		Tile& operator=(const Tile& other)     = default;
		Tile& operator=(Tile&& other) noexcept = default;

		// ======================================================================

		/// If true, resources will not spawn on this and player cannot navigate onto it
		PYTHON_PROP_REF_I(Tile, bool, isWater, false)

		/// Separately managed by data_manager
		PYTHON_PROP_I(Tile, Sprite*, sprite, nullptr)


		// ======================================================================
		// Renderer
		std::pair<Sprite*, Sprite::FrameT> OnRGetSprite(const UniqueDataBase* unique_data,
		                                                GameTickT game_tick) const override {
			return {sprite, 0};
		}

		bool OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
		                game::ChunkTileLayer* tile_layer) const override {
			return false;
		}

		// ======================================================================
		// Data
		void PostLoadValidate(const PrototypeManager& data_manager) const override;
	};

	inline void Tile::PostLoadValidate(const PrototypeManager&) const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not provided");
	}
}

#endif // DATA_PROTOTYPE_TILE_TILE_H
