#include "renderer/rendering/world_renderer.h"

#include <future>

#include "core/debug/execution_timer.h"

#include "game/world/world_manager.h"
#include "game/world/world_generator.h"

#include "renderer/rendering/mvp_manager.h"
#include "renderer/opengl/shader_manager.h"

/**
 * Places tiles from specified chunk into its position within the provided buffer
 * @param layer_index
 * @param buffer Holds positions for the textures
 * @param max_buffer_span Span of the buffer
 * @param max_buffer_height Height of the buffer
 * @param tiles Chunk tiles to render
 * @param buffer_x Beginning position of the chunk in buffer (tiles)
 * @param buffer_y Beginning position of the chunk in buffer (tiles)
 * @param func Function which returns internal name of sprite given a Chunk_tile <br>
 * return empty string to render transparency
 */
void buffer_load_chunk(const unsigned int layer_index, float* buffer,
                       const unsigned int max_buffer_span, const unsigned int max_buffer_height,
                       const jactorio::game::Chunk_tile* tiles,
                       const int buffer_x, const int buffer_y,
                       const jactorio::renderer::world_renderer::get_tile_prototype_func& func) {
	unsigned int start_x = 0;
	unsigned int start_y = 0;

	unsigned int end_x = 32;
	unsigned int end_y = 32;

	// Don't draw tiles out of frame (up, left)
	// This occurs when drawing partial chunks

	// Precompute the ranges for the loops ahead of time for PERFORMANCE
	if (buffer_x < 0)
		start_x = buffer_x * -1;
	if (buffer_x + 32 >= static_cast<int>(max_buffer_span))
		end_x = max_buffer_span - buffer_x;

	if (buffer_y < 0)
		start_y = buffer_y * -1;
	if (buffer_y + 32 >= static_cast<int>(max_buffer_height))
		end_y = max_buffer_height - buffer_y;

	// Iterate through and load tiles of a chunk into buffer
	for (unsigned int tile_y = start_y; tile_y < end_y; ++tile_y) {
		for (unsigned int tile_x = start_x; tile_x < end_x; ++tile_x) {
			auto tile = tiles[tile_y * 32 + tile_x];
			const unsigned int internal_id = func(tile, layer_index);

			const unsigned int buffer_offset =
				((buffer_y + tile_y) * max_buffer_span + (buffer_x + tile_x)) * 8;

			if (internal_id == 0) {
				// -1.f for the X position indicates drawing no sprites to fragment shader
				// Y position does not need to be specified
				buffer[buffer_offset + 0] = -1.f;
				// buffer[buffer_offset + 1] = -1.f;

				buffer[buffer_offset + 2] = -1.f;
				// buffer[buffer_offset + 3] = -1.f;

				buffer[buffer_offset + 4] = -1.f;
				// buffer[buffer_offset + 5] = -1.f;

				buffer[buffer_offset + 6] = -1.f;
				// buffer[buffer_offset + 7] = -1.f;
				continue;
			}
			
			const auto positions = jactorio::renderer::Renderer::get_spritemap_coords(internal_id);

			// Calculate the correct UV coordinates for multi-tile entities

			// The uv length of one tile
			float top_left_x = positions.top_left.x;
			float top_left_y = positions.top_left.y;

			float bottom_right_x = positions.bottom_right.x;
			float bottom_right_y = positions.bottom_right.y;

			// Split the sprite into sections and stretch over multiple tiles if this entity is multi tile
			auto& layer_tile = tile.get_layer(layer_index);
			if (layer_tile.multi_tile_span != 1) {
				const auto len_x =
					(positions.bottom_right.x - positions.top_left.x) / static_cast<float>(layer_tile.
						multi_tile_span);

				const auto len_y =
					(positions.bottom_right.y - positions.top_left.y) / static_cast<float>(layer_tile.
						multi_tile_height);

				const auto x_multiplier =
					static_cast<int>(layer_tile.multi_tile_index) % layer_tile.multi_tile_span;
				const auto y_multiplier =
					static_cast<int>(layer_tile.multi_tile_index) / layer_tile.
					multi_tile_span;  // This is correct, divide by width for both calculations

				// Opengl flips vertically, thus the y multiplier is inverted

				// bottom right
				bottom_right_x =
					positions.bottom_right.x - len_x * static_cast<float>(layer_tile.multi_tile_span - x_multiplier
						- 1);
				bottom_right_y = positions.bottom_right.y - len_y * y_multiplier;

				// top left
				top_left_x = positions.top_left.x + len_x * x_multiplier;
				top_left_y =
					positions.top_left.y + len_y * static_cast<float>(layer_tile.multi_tile_height - y_multiplier -
						1);
			}

			// bottom left
			buffer[buffer_offset + 0] = top_left_x;
			buffer[buffer_offset + 1] = bottom_right_y;

			// bottom right
			buffer[buffer_offset + 2] = bottom_right_x;
			buffer[buffer_offset + 3] = bottom_right_y;

			// top right
			buffer[buffer_offset + 4] = bottom_right_x;
			buffer[buffer_offset + 5] = top_left_y;

			// top left
			buffer[buffer_offset + 6] = top_left_x;
			buffer[buffer_offset + 7] = top_left_y;
		}
	}
}

void jactorio::renderer::world_renderer::draw_chunks(const Renderer* renderer, const unsigned int layer_index,
                                                     const int window_start_x, const int window_start_y,
                                                     const int chunk_start_x, const int chunk_start_y,
                                                     const unsigned int chunk_amount_x,
                                                     const unsigned int chunk_amount_y,
                                                     const get_tile_prototype_func& func,
                                                     const bool using_buffer1) {
	const unsigned short buffer_span = renderer->get_grid_size_x();
	const unsigned short buffer_height = renderer->get_grid_size_y();

	float* buffer;
	if (using_buffer1)
		buffer = renderer->render_layer.get_buf_uv().ptr;
	else
		buffer = renderer->render_layer2.get_buf_uv().ptr;

	for (unsigned int chunk_y = 0; chunk_y < chunk_amount_y; ++chunk_y) {
		const int chunk_y_offset = chunk_y * 32 + window_start_y;

		for (unsigned int chunk_x = 0; chunk_x < chunk_amount_x; ++chunk_x) {
			const int chunk_x_offset = chunk_x * 32 + window_start_x;

			const auto chunk = game::world_manager::get_chunk(chunk_start_x + chunk_x,
			                                                  chunk_start_y + chunk_y);

			// Generate chunk if non existent
			if (chunk == nullptr) {
				game::world_generator::queue_chunk_generation(
					chunk_start_x + chunk_x,
					chunk_start_y + chunk_y);
				continue;
			}


			// Don't try to render chunks which are out of view
			if (chunk_x_offset > buffer_span || chunk_y_offset > buffer_height)
				continue;

			// Load chunk into buffer
			game::Chunk_tile* tiles = chunk->tiles_ptr();

			buffer_load_chunk(layer_index,
			                  buffer, buffer_span, buffer_height,
			                  tiles,
			                  chunk_x_offset, chunk_y_offset,
			                  func);
		}
	}
}


void jactorio::renderer::world_renderer::render_player_position(
	Renderer* renderer, const float player_x, const float player_y) {
	// Player movement is in tiles
	// Every 32 tiles, shift 1 chunk
	// Remaining tiles are offset

	// The top left of the tile at player position will be at the center of the screen

	// On a 1920 x 1080 screen:
	// 960 pixels from left
	// 540 pixels form top
	// Right and bottom varies depending on tile size

	// Player position with decimal removed
	const auto position_x = static_cast<int>(player_x);
	const auto position_y = static_cast<int>(player_y);


	// How many chunks to offset based on player's position
	auto chunk_start_x = static_cast<int>(position_x / 32);
	auto chunk_start_y = static_cast<int>(position_y / 32);

	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus 32 to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	auto tile_start_x = static_cast<int>(position_x % 32 * -1);
	auto tile_start_y = static_cast<int>(position_y % 32 * -1);


	const auto tile_amount_x = renderer->get_grid_size_x();
	const auto tile_amount_y = renderer->get_grid_size_y();

	const auto chunk_amount_x = tile_amount_x / 32;
	const auto chunk_amount_y = tile_amount_y / 32;


	// Render the player position in the center of the screen
	chunk_start_x -= tile_amount_x / 2 / 32;
	tile_start_x += tile_amount_x / 2 % 32;

	chunk_start_y -= tile_amount_y / 2 / 32;
	tile_start_y += tile_amount_y / 2 % 32;

	{
		// ##################
		// View matrix

		// Negative moves window right and down

		// Decimal is used to shift the camera
		// Invert the movement to give the illusion of moving in the correct direction
		const float camera_offset_x =
			(player_x - position_x) * static_cast<float>(Renderer::tile_width) * -1;
		const float camera_offset_y =
			(player_y - position_y) * static_cast<float>(Renderer::tile_width) * -1;

		// Remaining pixel distance not covered by tiles and chunks are covered by the view matrix
		// to center pixel (For centering specification, see top of function)
		const auto window_width = Renderer::get_window_width();
		const auto window_height = Renderer::get_window_height();

		// Divide by 2 first to truncate decimals
		const auto view_transform = mvp_manager::get_view_transform();
		view_transform->x
			= static_cast<int>(window_width / 2 - (tile_amount_x / 2 * Renderer::tile_width))
			+ camera_offset_x;

		view_transform->y
			= static_cast<int>(window_height / 2 - (tile_amount_y / 2 * Renderer::tile_width))
			+ camera_offset_y;

		// Set view matrix
		mvp_manager::update_view_transform();
		// Set projection matrix
		renderer->update_tile_projection_matrix();
		update_shader_mvp();
	}

	EXECUTION_PROFILE_SCOPE(profiler, "World draw");

	const auto get_tile_proto_func = [](const game::Chunk_tile& chunk_tile, const unsigned int layer_index) {
		const auto& layer = chunk_tile.layers[layer_index];

		if (layer.sprite == nullptr)
			return 0u;

		return layer.sprite->internal_id;
	};

	// Rendering layers utilizes the following pattern looped
	// Prepare 1 - ASYNC
	// Wait 2
	// Update 2
	// Draw 2
	// -------------------
	// Prepare 2 - ASYNC
	// Wait 1
	// Update 1
	// Draw 1

	std::future<void> preparing_thread1;
	std::future<void> preparing_thread2 =
		std::async(std::launch::async, draw_chunks,
		           renderer, 0,
		           // - 64 to hide the 2extra chunk around the outside screen
		           tile_start_x - 64, tile_start_y - 64,
		           // 2 extra chunk in either direction ensure 
		           // window will always be filled with chunks regardless
		           // of chunk offset and window size
		           chunk_start_x - 2,
		           chunk_start_y - 2,
		           chunk_amount_x + 2 + 2,
		           chunk_amount_y + 2 + 2,
		           get_tile_proto_func, false);


	bool using_buffer1 = true;

	// Begin at index 1, since index 0 is handled above
	for (unsigned int layer_index = 1; layer_index < game::Chunk_tile::layer_count; ++layer_index) {
		// Prepare 1
		if (using_buffer1) {
			preparing_thread1 =
				std::async(std::launch::async, draw_chunks,
				           renderer, layer_index,
				           // - 64 to hide the 2extra chunk around the outside screen
				           tile_start_x - 64, tile_start_y - 64,
				           // 2 extra chunk in either direction ensure 
				           // window will always be filled with chunks regardless
				           // of chunk offset and window size
				           chunk_start_x - 2,
				           chunk_start_y - 2,
				           chunk_amount_x + 2 + 2,
				           chunk_amount_y + 2 + 2,
				           get_tile_proto_func, true);

			preparing_thread2.wait();

			renderer->render_layer2.g_update_data();
			renderer->render_layer2.g_buffer_bind();
		}
			// Prepare 2
		else {
			preparing_thread2 =
				std::async(std::launch::async, draw_chunks,
				           renderer, layer_index,
				           // - 64 to hide the 2extra chunk around the outside screen
				           tile_start_x - 64, tile_start_y - 64,
				           // 2 extra chunk in either direction ensure 
				           // window will always be filled with chunks regardless
				           // of chunk offset and window size
				           chunk_start_x - 2,
				           chunk_start_y - 2,
				           chunk_amount_x + 2 + 2,
				           chunk_amount_y + 2 + 2,
				           get_tile_proto_func, false);

			preparing_thread1.wait();

			renderer->render_layer.g_update_data();
			renderer->render_layer.g_buffer_bind();
		}

		renderer->g_draw();
		using_buffer1 = !using_buffer1;
	}

	// Wait for the final layer to draw
	if (using_buffer1) {
		preparing_thread2.wait();
		renderer->render_layer2.g_update_data();
		renderer->render_layer2.g_buffer_bind();
	}
	else {
		preparing_thread1.wait();
		renderer->render_layer.g_update_data();
		renderer->render_layer.g_buffer_bind();
	}

	renderer->g_draw();
}
