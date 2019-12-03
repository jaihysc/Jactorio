#include "renderer/rendering/world_renderer.h"

#include <future>

#include "core/data_type/unordered_map.h"
#include "game/world/world_manager.h"
#include "game/world/world_generator.h"
#include "renderer/rendering/mvp_manager.h"
#include "renderer/rendering/renderer_manager.h"
#include "core/logger.h"
#include "renderer/opengl/shader_manager.h"

namespace
{
	// Threads for generating new chunks, threading world gen prevents the renderer from being blocked
	// while generating
	std::atomic<int> world_gen_threads_count = 0;
	// Prevents constant erasing
	bool world_gen_threads_cleared = true;
	
	std::vector<std::future<void>> world_gen_threads;
	// Stores whether or not a chunk is being generated
	std::unordered_map<std::tuple<int, int>, bool,
	                   jactorio::core::hash<std::tuple<int, int>>> world_gen_chunks;


	// Tracks number of running buffer_load_chunk threads
	std::atomic<int> buffer_load_chunk_thread_counter = 0;

	/**
	 * Places tiles from specified chunk into its position within the provided buffer
	 * @param buffer Holds positions for the textures
	 * @param span Span of the buffer
	 * @param height Height of the buffer
	 * @param renderer Renderer which containing the spritemap positions for the tiles which will be drawn
	 * @param tiles Chunk tiles to render
	 * @param buffer_x Beginning position of the chunk in buffer (tiles)
	 * @param buffer_y Beginning position of the chunk in buffer (tiles)
	 * @param layer_index Prototype layer index to draw, empty will be drawn if tile does not contain
	 * a prototype at current layer
	 */
	void buffer_load_chunk(float* buffer, const unsigned int span, const unsigned int height,
	                       const jactorio::renderer::Renderer* renderer,
	                       const jactorio::game::Chunk_tile* tiles,
	                       const int buffer_x, const int buffer_y,
	                       const unsigned int layer_index) {
		
		// Iterate through and draw tiles of a chunk
		for (int tile_y = 0; tile_y < 32; ++tile_y) {
			const unsigned int y_offset = buffer_y + tile_y;
			if (y_offset >= height)
				continue;
			
			for (int tile_x = 0; tile_x < 32; ++tile_x) {
				const unsigned int x_offset = buffer_x + tile_x;
				
				if (x_offset >= span)
					continue;

				// Load sprite positions
				auto& prototypes_vector = tiles[tile_y * 32 + tile_x].
					tile_prototypes;

				const unsigned int buffer_offset = 
					(y_offset * span + x_offset) * 8;

				if (layer_index >= prototypes_vector.size()) {
					// -1.f position indicates drawing no sprites to fragment shader
					buffer[buffer_offset + 0] = -1.f;
					buffer[buffer_offset + 1] = -1.f;

					buffer[buffer_offset + 2] = -1.f;
					buffer[buffer_offset + 3] = -1.f;

					buffer[buffer_offset + 4] = -1.f;
					buffer[buffer_offset + 5] = -1.f;

					buffer[buffer_offset + 6] = -1.f;
					buffer[buffer_offset + 7] = -1.f;
				}
				else {
					const auto positions = renderer->get_sprite_spritemap_coords(
						prototypes_vector[layer_index]->name);
					
					buffer[buffer_offset + 0] = positions.bottom_left.x;
					buffer[buffer_offset + 1] = positions.bottom_left.y;

					buffer[buffer_offset + 2] = positions.bottom_right.x;
					buffer[buffer_offset + 3] = positions.bottom_right.y;

					buffer[buffer_offset + 4] = positions.top_right.x;
					buffer[buffer_offset + 5] = positions.top_right.y;

					buffer[buffer_offset + 6] = positions.top_left.x;
					buffer[buffer_offset + 7] = positions.top_left.y;
				}
			}
		}

		--buffer_load_chunk_thread_counter;
	}
}

void jactorio::renderer::world_renderer::draw_chunks(const Renderer& renderer,
                                                     const int window_start_x,
                                                     const int window_start_y,
                                                     const int chunk_start_x,
                                                     const int chunk_start_y,
                                                     const unsigned int chunk_amount_x,
                                                     const unsigned int chunk_amount_y,
                                                     const unsigned int layer_index) {
	// Clear world gen threads once the vector is empty, this is faster than deleting it one by one
	if (!world_gen_threads_cleared && world_gen_threads_count == 0) {
		LOG_MESSAGE(debug, "Chunk generation buffer cleared");
		world_gen_threads.clear();
		world_gen_chunks.clear();
		world_gen_threads_cleared = true;
	}
	
	// Render chunks
	const unsigned short buffer_span = renderer.get_grid_size_x();
	const unsigned short buffer_height = renderer.get_grid_size_y();

	auto* buffer = renderer.get_texture_grid_buffer();

	std::vector<std::future<void>> chunk_load_threads;
	chunk_load_threads.reserve(static_cast<unsigned long long>(chunk_amount_x) * chunk_amount_y);
	
	for (int chunk_y = 0; chunk_y < chunk_amount_y; ++chunk_y) {
		const unsigned int chunk_y_offset = chunk_y * 32 + window_start_y;
		

		for (int chunk_x = 0; chunk_x < chunk_amount_x; ++chunk_x) {
			const unsigned int chunk_x_offset = chunk_x * 32 + window_start_x;

			auto chunk = game::world_manager::get_chunk(chunk_start_x + chunk_x,
			                                            chunk_start_y + chunk_y);

			// Generate chunk if non existent
			if (chunk == nullptr) {
				const auto chunk_key = std::tuple<int, int>
					{ chunk_start_x + chunk_x, chunk_start_y + chunk_y };
				
				// Is the chunk already under generation
				if (world_gen_chunks.find(chunk_key) != world_gen_chunks.end())
					continue;

				
				++world_gen_threads_count;
				world_gen_threads_cleared = false;
				
				world_gen_chunks[chunk_key] = true;

				// Chunk is asynchronously generated
				world_gen_threads.emplace_back(
					std::async(std::launch::async,
					           game::world_generator::generate_chunk,
					           chunk_start_x + chunk_x,
					           chunk_start_y + chunk_y,
					           &world_gen_threads_count)
				);
				continue;
			}

			game::Chunk_tile* tiles = chunk->tiles_ptr();

			// Load chunk into buffer
			++buffer_load_chunk_thread_counter;
			chunk_load_threads.emplace_back(
				std::async(std::launch::async,
				           buffer_load_chunk,
				           buffer, buffer_span, buffer_height,
				           &renderer, tiles,
				           chunk_x_offset, chunk_y_offset,
				           layer_index)
			);
		}
	}

	// Render - Wait for all threads to first finish
	while (buffer_load_chunk_thread_counter != 0)
		;

	renderer.update_texture_grid_buffer();
}


void jactorio::renderer::world_renderer::render_player_position(Renderer* renderer) {
	// Player movement is in pixels,
	//		if player has moved a tile's width, the tile shifts
	//		if player has moved a chunk's width, the chunk shifts
	// Otherwise only the camera shifts

	const auto tile_width = renderer->tile_width;
	
	// 32 is the number of tiles in a chunk
	const auto chunk_width = 32 * tile_width;
	
	const long long position_x = player_position_x;
	const long long position_y = player_position_y;

	
	// Tile transitions
	// Pixels not wide enough to form a tile is used to shift the camera
	const auto camera_offset_x = position_x % tile_width;
	const auto camera_offset_y = position_y % tile_width;

	
	const auto view_transform = mvp_manager::get_view_transform();
	// Invert the camera to give the illusion of moving in the correct direction
	view_transform->x = camera_offset_x * -1;
	view_transform->y = camera_offset_y * -1;
	
	// How many chunks to offset based on player's position
	const auto chunk_offset_x = static_cast<int>(position_x / chunk_width);
	const auto chunk_offset_y = static_cast<int>(position_y / chunk_width);

	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus 32 to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	const auto tile_offset_x = static_cast<int>(position_x / tile_width % 32 * -1);
	const auto tile_offset_y = static_cast<int>(position_y / tile_width % 32 * -1);

	// Set view matrix
	mvp_manager::update_view_transform();
	// Set projection matrix
	renderer->update_tile_projection_matrix();
	update_shader_mvp();

	for (int max_index = 0; max_index < renderer_manager::prototype_layer_count; ++max_index) {
		draw_chunks(*renderer,
		            // - 32 to hide the 1 extra chunk around the outside screen
		            tile_offset_x - 32, tile_offset_y - 32,
		            // 1 extra chunk in either direction ensure 
		            // window will always be filled with chunks regardless
		            // of chunk offset and window size
		            chunk_offset_x - 1,
		            chunk_offset_y - 1,
		            renderer->get_grid_size_x() / 32 + 2 + 1,
		            renderer->get_grid_size_y() / 32 + 2 + 1,
		            max_index);
		renderer->draw(glm::vec3(0, 0, 0));
	}
}
