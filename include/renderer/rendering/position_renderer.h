#ifndef RENDERER_RENDERING_POSITION_RENDERER_H
#define RENDERER_RENDERING_POSITION_RENDERER_H

#include "renderer/rendering/renderer.h"

/**
 * Renders items using positions in the world <br>
 * Examples include: trees, item entities
 */
namespace jactorio::renderer::position_renderer
{
	// Within game::Chunk, store a vector for positioned items (trees, items, etc)
	// The tile renderer is obviously faster since everything belongs to a known count of tiles
	//
	// The position stored by each element within the vector should be a float offset from the top left of the chunk
	// where 1 = 1 tile
	// Each element is behaviorally identical to Chunk_tile_layer without multi tile obviously,
	// make another class to use which the current Chunk_tile_layer will inherit from (without the multi tile stuff)
	//
	// I also need to store the size of these sprites, they will store their tile span X, Y

	// WHen items are on the ground, they exist as entities within their respective chunk
	// Items on transport belts however:
	//		Transport belt connected together are group by "segments" assigned by an id
	//		The segment holds all the items within it (Therefore no need to move item entities between chunks)

	/**
	 * Renders all object layers in ascending layer order
	 */
	void render_object_layers(Renderer* renderer, float player_x, float player_y);
}

#endif // RENDERER_RENDERING_POSITION_RENDERER_H
