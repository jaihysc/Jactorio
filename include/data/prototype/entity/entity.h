#ifndef DATA_PROTOTYPE_ENTITY_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_ENTITY_H

#include "jactorio.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/item/item.h"

namespace jactorio::data
{
	/**
	 * Placeable items in the world
	 */
	class Entity : public Prototype_base
	{
		/**
		 * Item when entity is picked up <br>
		 * Naming scheme should be <localized name of entity>-item
		 */
		Item* item_;

	public:
		Entity()
			: item_(nullptr), sprite(nullptr), tile_width(1), tile_height(1), rotatable(false) {
		}

		~Entity() override = default;


		/**
		 * Sprite drawn when placed in the world
		 */
		PYTHON_PROP(Entity, Sprite*, sprite)

		
		// Number of tiles this entity spans
		PYTHON_PROP_REF(Entity, unsigned short, tile_width)
		PYTHON_PROP_REF(Entity, unsigned short, tile_height)
		
		// Can be rotated by player?
		PYTHON_PROP_REF(Entity, bool, rotatable)

		// Item
		J_NODISCARD Item* get_item() const {
			return item_;
		}
		
		Entity* set_item(Item* item) {
			item->entity_prototype = this;
			this->item_ = item;
			
			return this;
		}

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
	};


}

#endif // DATA_PROTOTYPE_ENTITY_ENTITY_H
