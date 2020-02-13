#ifndef DATA_PROTOTYPE_ENTITY_TRANSPORT_BELT_H
#define DATA_PROTOTYPE_ENTITY_TRANSPORT_BELT_H

#include "transport_line.h"

namespace jactorio::data
{
	class Transport_belt final : public Transport_line
	{
		void on_build(game::Chunk_tile_layer* tile_layer) const override {
		}
	};
}

#endif // DATA_PROTOTYPE_ENTITY_TRANSPORT_BELT_H
