//
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
//

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H

#include "core/float_math.h"
#include "core/data_type.h"
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
		 * For Python API use only!
		 * Number of tiles traveled by each item on the belt per tick
		 */
		PYTHON_PROP_I(Transport_line, float, speed_float, 0.01);

		/**
		 * Number of tiles traveled by each item on the belt per tick
		 */
		transport_line_offset speed;

		// Deletes the internal structure behind transport lines, NOT the entities!
		void delete_unique_data(void* ptr) const override {
			delete static_cast<game::Transport_line_segment*>(ptr);
		}


		void post_load() override {
			// Convert floating point speed to fixed precision decimal speed
			speed = transport_line_offset (speed_float);
		}

		void post_load_validate() const override {
			J_DATA_ASSERT(speed_float > 0.001, "Transport line speed below minimum 0.001");
			// Cannot exceed item_width because of limitations in the logic
			J_DATA_ASSERT(speed_float < 0.25, "Transport line speed equal or above maximum of 0.25");
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_TRANSPORT_TRANSPORT_LINE_H
