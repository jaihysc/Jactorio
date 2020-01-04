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
		// Item when entity is picked up
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

		J_NODISCARD Item* get_item() const {
			return item_;
		}
		
		Entity* set_item(Item* item) {
			item->entity_prototype = this;
			this->item_ = item;
			
			return this;
		}
	};


}

#endif // DATA_PROTOTYPE_ENTITY_ENTITY_H
