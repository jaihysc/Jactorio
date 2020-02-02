#ifndef DATA_PROTOTYPE_ENTITY_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_ENTITY_H

#include "jactorio.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/item/item.h"
#include "game/world/chunk_tile_layer.h"

namespace jactorio::data
{
	// Unique per entity placed in the world
	struct Entity_data
	{
	};
	
	/**
	 * Placeable items in the world
	 */
	class Entity : public Prototype_base
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
		// Events

		void post_load_validate() const override;

		/**
		 * Entity was build in the world
		 * @param tile_layer Tile layer which the entity was built on
		 */
		virtual void on_build(game::Chunk_tile_layer* tile_layer) const {
		}

		/**
		 * Entity was picked up from a built state
		 */
		virtual void on_remove() const {
		}

		/**
		 * Displays the menu associated with itself with the provided data
		 */
		virtual void on_show_gui(game::Chunk_tile_layer* tile_layer) const {
		}
	};

	inline void Entity::post_load_validate() const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified")
		J_DATA_ASSERT(pickup_time >= 0, "Pickup time must be 0 or positive")
	}
}

#endif // DATA_PROTOTYPE_ENTITY_ENTITY_H
