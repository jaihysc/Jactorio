#ifndef DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H

#include "health_entity.h"

namespace jactorio::data
{
	struct Container_entity_data : Entity_data
	{
		explicit Container_entity_data(const uint16_t inventory_size) {
			inventory = new item_stack[inventory_size];
		}

		~Container_entity_data() {
			delete[] inventory;
		}

		Container_entity_data(const Container_entity_data& other) = delete;
		Container_entity_data(Container_entity_data&& other) noexcept = delete;
		Container_entity_data& operator=(const Container_entity_data& other) = delete;
		Container_entity_data& operator=(Container_entity_data&& other) noexcept = delete;

		item_stack* inventory;
	};
	
	/**
	 * An entity with an inventory, such as a chest
	 */
	class Container_entity final : public Health_entity
	{
	public:
		Container_entity()
			: inventory_size(0) {
		}

		PYTHON_PROP_REF(Container_entity, uint16_t, inventory_size)


		void delete_unique_data(void* ptr) const override;

		void* copy_unique_data(void* ptr) const override;

		// Events

		void on_build(game::Chunk_tile_layer* tile_layer) const override;

		void on_show_gui(game::Chunk_tile_layer* tile_layer) const override;
	};
}

#endif // DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
