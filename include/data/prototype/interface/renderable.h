// 
// renderable.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/15/2020
// Last modified: 03/18/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#pragma once

#include "data/prototype/sprite.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile_layer.h"

namespace jactorio::data
{
	///
	/// \brief Inherit to allow drawing portions of a sprite
	struct Renderable_data
	{
		Renderable_data() = default;

		Renderable_data(const uint16_t set, const uint16_t frame)
			: set(set), frame(frame) {
		}

		uint16_t set = 0;
		uint16_t frame = 0;
	};

	///
	/// \brief Enables usage by renderer
	class Renderable
	{
	protected:
		Renderable() = default;
		virtual ~Renderable() = default;

		Renderable(const Renderable& other) = default;
		Renderable(Renderable&& other) noexcept = default;
		Renderable& operator=(const Renderable& other) = default;
		Renderable& operator=(Renderable&& other) noexcept = default;

	public:
		///
		/// \brief Called by the renderer when it wants the sprite associated with this entity
		J_NODISCARD virtual Sprite* on_r_get_sprite(void* unique_data) const = 0;

		///
		/// \brief Displays the menu associated with itself with the provided data
		virtual void on_r_show_gui(game::Player_data& player_data, game::Chunk_tile_layer* tile_layer) const = 0;
	};
}

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
