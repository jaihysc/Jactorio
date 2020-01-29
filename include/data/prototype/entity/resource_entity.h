#ifndef DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#define DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H

#include "data/prototype/entity/entity.h"

namespace jactorio::data
{
	class Resource_entity final : public Entity
	{
	public:
		Resource_entity()
			: product(nullptr), resource_amount(1) {
		}

		~Resource_entity() override = default;

		Resource_entity(const Resource_entity& other) = default;
		Resource_entity(Resource_entity&& other) noexcept = default;
		
		Resource_entity& operator=(const Resource_entity& other) = default;
		Resource_entity& operator=(Resource_entity&& other) noexcept = default;


		/**
		 * Item given when this resource is extracted
		 */
		PYTHON_PROP(Resource_entity, Item*, product)

		
		// Unique per tile

		/**
		 * Amount of product which can still be extracted from this tile
		 */
		uint16_t resource_amount;
	};
}

#endif // DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
