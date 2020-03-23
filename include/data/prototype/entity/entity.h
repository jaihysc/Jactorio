// 
// entity.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 03/23/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_ENTITY_H
#pragma once

#include <utility>

#include "jactorio.h"

#include "data/prototype/interface/renderable.h"
#include "data/prototype/item/item.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile_layer.h"
#include "game/world/world_data.h"

namespace jactorio::data
{
	// Unique per entity placed in the world
	struct Entity_data : Renderable_data
	{
	};

	/**
	 * Placeable items in the world
	 */
	class Entity : public Prototype_base, public Renderable
	{
		/**
		 * Item when entity is picked up <br>
		 * Naming scheme should be <localized name of entity>-item
		 */
		Item* item_ = nullptr;

	public:
		Entity() = default;

		~Entity() override = default;

		Entity(const Entity& other) = default;
		Entity(Entity&& other) noexcept = default;

		Entity& operator=(const Entity& other) = default;
		Entity& operator=(Entity&& other) noexcept = default;

		/**
		 * Sprite drawn when placed in the world
		 */
		PYTHON_PROP_I(Entity, Sprite*, sprite, nullptr)


		// Number of tiles this entity spans
		PYTHON_PROP_REF_I(Entity, unsigned short, tile_width, 1)
		PYTHON_PROP_REF_I(Entity, unsigned short, tile_height, 1)

		// Can be rotated by player?
		PYTHON_PROP_REF_I(Entity, bool, rotatable, false)
		// Can be placed by player?
		PYTHON_PROP_REF_I(Entity, bool, placeable, true)

		// Item
		J_NODISCARD Item* get_item() const {
			return item_;
		}

		Entity* set_item(Item* item) {
			item->entity_prototype = this;
			this->item_ = item;

			return this;
		}

		/**
		 * Seconds to pickup entity
		 */
		PYTHON_PROP_REF_I(Entity, float, pickup_time, 1);


		void post_load_validate() const override;

		// ======================================================================
		// Localized names

		// Override the default setter to also set for the item associated with this
		void set_localized_name(const std::string& localized_name) override {
			this->localized_name_ = localized_name;
			if (item_ != nullptr)
				item_->set_localized_name(localized_name);
		}

		void set_localized_description(const std::string& localized_description) override {
			this->localized_description_ = localized_description;
			if (item_ != nullptr)
				item_->set_localized_description(localized_description);
		}

		// ======================================================================
		// Renderer events

		Sprite* on_r_get_sprite(void* unique_data) const override {
			return this->sprite;
		}

		// ======================================================================
		// Game events

		///
		/// \brief Entity was build in the world
		virtual void on_build(game::World_data& world_data, std::pair<int, int> world_coords,
		                      game::Chunk_tile_layer& tile_layer, uint16_t frame,
		                      placementOrientation orientation) const {
		}

		///
		/// \brief Entity was picked up from a built state, called BEFORE the entity has been removed
		virtual void on_remove(game::World_data& world_data, std::pair<int, int> world_coords,
		                       game::Chunk_tile_layer& tile_layer) const {
		}
	};

	inline void Entity::post_load_validate() const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified")
		J_DATA_ASSERT(pickup_time >= 0, "Pickup time must be 0 or positive")
	}
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_ENTITY_H
