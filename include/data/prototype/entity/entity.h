// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_ENTITY_H
#pragma once

#include <utility>

#include "jactorio.h"

#include "data/prototype/interface/renderable.h"
#include "data/prototype/item/item.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace jactorio::data
{
	///
	/// \brief Unique per entity placed in the world
	struct EntityData : RenderableData
	{
	};

	///
	/// \brief Placeable items in the world
	class Entity : public PrototypeBase, public Renderable, public Rotatable
	{
	public:
		Entity() = default;

		~Entity() override = default;

		Entity(const Entity& other)     = default;
		Entity(Entity&& other) noexcept = default;

		Entity& operator=(const Entity& other)     = default;
		Entity& operator=(Entity&& other) noexcept = default;

	private:

		/// \brief Item when entity is picked up
		Item* item_ = nullptr;

	public:
		/// Sprite drawn when placed in the world
		/// \remark For rotatable entities, this serves as the north sprite if multiple sprites are used
		PYTHON_PROP_I(Entity, Sprite*, sprite, nullptr)


		// Number of tiles this entity spans
		PYTHON_PROP_REF_I(Entity, uint8_t, tileWidth, 1)
		PYTHON_PROP_REF_I(Entity, uint8_t, tileHeight, 1)

		// Can be rotated by player?
		PYTHON_PROP_REF_I(Entity, bool, rotatable, false)
		// Can be placed by player?
		PYTHON_PROP_REF_I(Entity, bool, placeable, true)

		// Item
		J_NODISCARD Item* GetItem() const {
			return item_;
		}

		Entity* SetItem(Item* item) {
			item->entityPrototype = this;
			this->item_            = item;

			return this;
		}

		/**
		 * Seconds to pickup entity
		 */
		PYTHON_PROP_REF_I(Entity, float, pickupTime, 1);


		void PostLoadValidate() const override;

		// ======================================================================
		// Localized names

		// Override the default setter to also set for the item associated with this
		void SetLocalizedName(const std::string& localized_name) override {
			this->localizedName_ = localized_name;
			if (item_ != nullptr)
				item_->SetLocalizedName(localized_name);
		}

		void SetLocalizedDescription(const std::string& localized_description) override {
			this->localizedDescription_ = localized_description;
			if (item_ != nullptr)
				item_->SetLocalizedDescription(localized_description);
		}

		// ======================================================================
		// Renderer events

		std::pair<Sprite*, Sprite::FrameT> OnRGetSprite(const UniqueDataBase* unique_data,
		                                                        GameTickT game_tick) const override {
			return {this->sprite, 0};
		}

		// ======================================================================
		// Game events

		///
		/// \brief Entity was build in the world
		virtual void OnBuild(game::WorldData& world_data,
		                     const game::WorldData::WorldPair& world_coords,
		                     game::ChunkTileLayer& tile_layer,
		                     Orientation orientation) const = 0;

		///
		/// \brief Returns true if itself can be built at the specified world_coords being its top left
		/// \return true if can be built
		J_NODISCARD virtual bool OnCanBuild(const game::WorldData& world_data,
		                                    const game::WorldData::WorldPair& world_coords) const {
			return true;
		}


		///
		/// \brief Entity was picked up from a built state, called BEFORE the entity has been removed
		virtual void OnRemove(game::WorldData& world_data,
		                      const game::WorldData::WorldPair& world_coords,
		                      game::ChunkTileLayer& tile_layer) const = 0;

		///
		/// \brief A neighbor of this prototype in the world was updated
		/// \param world_data 
		/// \param emit_world_coords Coordinates of the prototype which is EMITTING the update 
		/// \param receive_world_coords Layer of the prototype RECEIVING the update 
		/// \param emit_orientation Orientation to the prototype EMITTING the update 
		virtual void OnNeighborUpdate(game::WorldData& world_data,
		                              const game::WorldData::WorldPair& emit_world_coords,
		                              const game::WorldData::WorldPair& receive_world_coords,
		                              Orientation emit_orientation) const {
		}
	};

	inline void Entity::PostLoadValidate() const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified")
		J_DATA_ASSERT(pickupTime >= 0, "Pickup time must be 0 or positive")
	}
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_ENTITY_H
