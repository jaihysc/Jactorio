#include <game/world/tile.h>

// A World_chunk is a 32 x 32 grid of tiles

class World_chunk
{
public:
	World_tile* tiles[32][32];
};