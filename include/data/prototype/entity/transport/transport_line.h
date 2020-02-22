#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H

#include "data/prototype/entity/health_entity.h"

#include "game/logic/transport_line_structure.h"

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
		PYTHON_PROP_I(Transport_line, float, speed, 0.01);


		// void post_load_validate() const override;

		// Deletes the internal structure behind transport lines, NOT the entities!
		void delete_unique_data(void* ptr) const override {
			delete static_cast<game::Transport_line_segment*>(ptr);
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
