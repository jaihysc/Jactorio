// 
// resource_entity.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/07/2020
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#pragma once

#include "data/prototype/entity/entity.h"

namespace jactorio::data
{
	// Unique per resource entity placed
	struct Resource_entity_data : Entity_data
	{
		Resource_entity_data() = default;

		explicit Resource_entity_data(const uint16_t resource_amount)
			: resource_amount(resource_amount) {
		}

		/**
		 * Amount of product which can still be extracted from this tile
		 */
		uint16_t resource_amount;
	};

	class Resource_entity final : public Entity
	{
	public:
		Resource_entity() {
			// Resource entities can never be placed
			this->placeable = false;
		}

		~Resource_entity() override = default;

		Resource_entity(const Resource_entity& other) = default;
		Resource_entity(Resource_entity&& other) noexcept = default;

		Resource_entity& operator=(const Resource_entity& other) = default;
		Resource_entity& operator=(Resource_entity&& other) noexcept = default;


		void delete_unique_data(void* ptr) const override {
			delete static_cast<Resource_entity_data*>(ptr);
		}

		void* copy_unique_data(void* ptr) const override {
			const auto other = new Resource_entity_data();
			*other = *static_cast<Resource_entity_data*>(ptr);
			return other;
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
