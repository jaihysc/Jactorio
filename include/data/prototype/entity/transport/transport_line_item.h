#ifndef DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_ITEM_H
#define DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_ITEM_H

#include "data/prototype/item/item.h"
#include "data/prototype/entity/entity.h"

namespace jactorio::data
{
	struct Transport_line_item_data : Entity_data
	{
		/**
		 * Item move direction
		 */
		enum class move_dir
		{
			stop = 0,
			up,
			right,
			down,
			left
		};

		move_dir direction;
	};
	
	class Transport_line_item final : public Item_base
	{
	public:

		// Not overriding here is intentional, so a copy of a data::Item does not need to be made
		// for transport lines. Instead it can be cast to a Transport_line_item and it will call the
		// delete_unique_data for Transport_line_item
		void delete_unique_data(void* ptr) const {
			delete static_cast<Transport_line_item_data*>(ptr);
		}
	};
}

#endif // DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_ITEM_H
