// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
#pragma once

#include "core/data_type.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio
{
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
	struct RenderableData : UniqueDataBase
	{
		RenderableData() = default;

		RenderableData(const Sprite::SetT set)
			: set(set) {
		}

		Sprite::SetT set = 0;

		///
		/// \param layer
		/// \param x_offset Pixels to top left of current tile
		/// \param y_offset Pixels to top left of current tile
		virtual void OnDrawUniqueData(renderer::RendererLayer& layer,
		                              float x_offset, float y_offset) const {
		}
	};

	///
	/// \brief Enables usage by renderer
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
		/// \brief Called by the renderer when it wants the sprite and frame within the sprite
		virtual std::pair<Sprite*, Sprite::FrameT> OnRGetSprite(const UniqueDataBase* unique_data,
		                                                        GameTickT game_tick) const = 0;

		///
		/// \brief Displays the menu associated with itself with the provided data
		virtual bool OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const = 0;
	};
}

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_RENDERABLE_H
