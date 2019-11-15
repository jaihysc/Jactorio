#include "renderer/rendering/world_renderer.h"

#include "game/world/world_manager.h"
#include "renderer/rendering/mvp_manager.h"

void jactorio::renderer::world_renderer::render_player_position(Renderer* renderer) {
	// Player movement is in pixels,
	//		if player has moved a tile's width, the tile shifts
	//		if player has moved a chunk's width, the chunk shifts
	// Otherwise only the camera shifts

	// 32 is the width of chunks
	const auto chunk_width = 32 * Renderer::tile_width;
	const long long position_x = player_position_x / -1;
	const long long position_y = player_position_y / -1;

	// How many chunks to offset based on player's position
	const auto chunk_offset_x =
		static_cast<long long>(position_x / chunk_width);
	const auto chunk_offset_y =
		static_cast<long long>(position_y / chunk_width);

	// Player has not moved an entire chunk's width yet, offset the tiles
	const auto tile_offset_x = 
		static_cast<long long>(position_x / Renderer::tile_width);
	const auto tile_offset_y =
		static_cast<long long>(position_y / Renderer::tile_width);

	game::world_manager::draw_chunks(*renderer,
	                                 tile_offset_x, tile_offset_y,
	                                 chunk_offset_x, chunk_offset_y,

	                                 // 1 extra chunk so window will always be filled with chunks regardless of window size
	                                 renderer->get_grid_size_x() / 32 + 1, 
	                                 renderer->get_grid_size_y() / 32 + 1);


	// Pixels not wide enough to form a tile is used to shift the camera
	const auto camera_offset_x = position_x % Renderer::tile_width;
	const auto camera_offset_y = position_y % Renderer::tile_width;

	const auto view_transform = mvp_manager::get_view_transform();
	view_transform->x = camera_offset_x;
	view_transform->y = camera_offset_y;
}
