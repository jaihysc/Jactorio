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

		class Chunk;
	}

	namespace data
	{
		class Sprite;
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
	struct RenderableData : UniqueDataBase
	{
		using SetT = uint16_t;
		using FrameT = uint16_t;

		RenderableData() = default;

		RenderableData(const SetT set)
			: set(set) {
		}

		SetT set = 0;

		///
		/// \param layer
		/// \param x_offset Pixels to top left of current tile
		/// \param y_offset Pixels to top left of current tile
		virtual void OnDrawUniqueData(renderer::RendererLayer& layer,
		                              float x_offset, float y_offset) {
		}
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
		virtual std::pair<Sprite*, RenderableData::FrameT> OnRGetSprite(UniqueDataBase* unique_data,
		                                                                GameTickT game_tick) const = 0;

		///
		/// \brief Displays the menu associated with itself with the provided data
		virtual void OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const = 0;
	};
}

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
