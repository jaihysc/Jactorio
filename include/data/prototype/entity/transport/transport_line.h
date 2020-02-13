#ifndef DATA_PROTOTYPE_ENTITY_TRANSPORT_LINE_H
#define DATA_PROTOTYPE_ENTITY_TRANSPORT_LINE_H

#include "data/prototype/entity/health_entity.h"

namespace jactorio::data
{
	struct Transport_line_data : Health_entity_data
	{
	};

	/**
	 * Abstract class for all everything which moves items (belts, underground belts, splitters, etc)
	 */
	class Transport_line : public Health_entity
	{
	protected:
		Transport_line() = default;

	public:
		/**
		 * Number of tiles traveled by each item on the belt per tick
		 */
		PYTHON_PROP_I(Transport_line, float, speed, 0.016);

		
		// void post_load_validate() const override;
	};
}

#endif // DATA_PROTOTYPE_ENTITY_TRANSPORT_LINE_H
