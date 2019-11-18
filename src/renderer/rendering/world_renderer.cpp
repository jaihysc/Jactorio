#include "renderer/rendering/world_renderer.h"

#include "game/world/world_manager.h"
#include "renderer/rendering/mvp_manager.h"

void jactorio::renderer::world_renderer::render_player_position(Renderer* renderer) {
	// Player movement is in pixels,
	//		if player has moved a tile's width, the tile shifts
	//		if player has moved a chunk's width, the chunk shifts
	// Otherwise only the camera shifts

	// 32 is the number of tiles in a chunk
	const auto chunk_width = 32 * Renderer::tile_width;
	
	const long long position_x = player_position_x;
	const long long position_y = player_position_y;

	
	// How many chunks to offset based on player's position
	const auto chunk_offset_x = static_cast<int>(position_x / chunk_width);
	const auto chunk_offset_y = static_cast<int>(position_y / chunk_width);

	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus 32 to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	const auto tile_offset_x = static_cast<int>(position_x / Renderer::tile_width % 32 * -1);
	const auto tile_offset_y = static_cast<int>(position_y / Renderer::tile_width % 32 * -1);
	
	game::world_manager::draw_chunks(*renderer,
	                                 // - 64 to hide the 2 extra chunks around the outside screen
	                                 tile_offset_x - 64, tile_offset_y - 64,
	                                 // 2 extra chunks in either direction ensure 
	                                 // window will always be filled with chunks regardless
	                                 // of chunk offset and window size
	                                 chunk_offset_x - 2, 
	                                 chunk_offset_y - 2,
	                                 renderer->get_grid_size_x() / 32 + 2 + 2, 
	                                 renderer->get_grid_size_y() / 32 + 2 + 2);


	// Tile transitions
	// Pixels not wide enough to form a tile is used to shift the camera
	const auto camera_offset_x = position_x % Renderer::tile_width;
	const auto camera_offset_y = position_y % Renderer::tile_width;

	const auto view_transform = mvp_manager::get_view_transform();
	// Invert the camera to give the illusion of moving in the correct direction
	view_transform->x = camera_offset_x * -1;
	view_transform->y = camera_offset_y * -1;
}
