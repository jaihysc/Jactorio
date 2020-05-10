// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#pragma once

#include "core/data_type.h"
#include "data/prototype/prototype_base.h"

namespace jactorio
{
	namespace game
	{
		class PlayerData;
		class ChunkTileLayer;
	}

	namespace data
	{
		class Sprite;
	}
}

namespace jactorio::data
{
	///
	/// \brief Inherit to allow drawing portions of a sprite
	struct RenderableData : UniqueDataBase
	{
		using set_t = uint16_t;
		using frame_t = uint16_t;

		RenderableData() = default;

		RenderableData(const set_t set)
			: set(set) {
		}

		set_t set = 0;
	};

	///
	/// \brief Enables usage by renderer
	class Renderable
	{
	protected:
		Renderable()          = default;
		virtual ~Renderable() = default;

		Renderable(const Renderable& other)                = default;
		Renderable(Renderable&& other) noexcept            = default;
		Renderable& operator=(const Renderable& other)     = default;
		Renderable& operator=(Renderable&& other) noexcept = default;

	public:
		///
		/// \brief Called by the renderer when it wants the sprite and frame within the sprite
		J_NODISCARD virtual std::pair<Sprite*, RenderableData::frame_t> OnRGetSprite(UniqueDataBase* unique_data,
		                                                                             GameTickT game_tick) const = 0;

		///
		/// \brief Displays the menu associated with itself with the provided data
		virtual void OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const = 0;
	};
}

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
