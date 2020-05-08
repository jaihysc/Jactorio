// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/06/2019

#ifndef DATA_PROTOTYPE_TILE_TILE_H
#define DATA_PROTOTYPE_TILE_TILE_H

#include "data/prototype/interface/renderable.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	///
	/// \brief Tiles which are auto-placed by the world generator <br>
	/// Configure how the world generator places these tiles with a Noise_layer
	class Tile : public Prototype_base, public Renderable
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

		Tile(const Tile& other) = default;
		Tile(Tile&& other) noexcept = default;
		Tile& operator=(const Tile& other) = default;
		Tile& operator=(Tile&& other) noexcept = default;

		// ======================================================================

		/// If true, resources will not spawn on this and player cannot navigate onto it
		PYTHON_PROP_REF_I(Tile, bool, is_water, false)

		/// Separately managed by data_manager
		PYTHON_PROP_I(Tile, Sprite*, sprite, nullptr)


		// ======================================================================
		// Renderer
		std::pair<Sprite*, Renderable_data::frame_t> on_r_get_sprite(Unique_data_base* unique_data,
																	 game_tick_t game_tick) const override {
			return {sprite, 0};
		}

		void on_r_show_gui(game::Player_data& player_data, game::Chunk_tile_layer* tile_layer) const override {
		}

		// ======================================================================
		// Data
		void post_load_validate() const override;
	};

	inline void Tile::post_load_validate() const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not provided");
	}
}

#endif // DATA_PROTOTYPE_TILE_TILE_H
