// 
// container_entity.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#pragma once

#include "health_entity.h"

namespace jactorio::data
{
	struct Container_entity_data : Health_entity_data
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

		void on_show_gui(game::Player_data& player_data, game::Chunk_tile_layer* tile_layer) const override;
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
