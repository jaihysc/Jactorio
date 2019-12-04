#include "renderer/rendering/world_renderer.h"

#include <future>

#include "core/debug/execution_timer.h"

#include "game/world/world_manager.h"
#include "game/world/world_generator.h"

#include "renderer/rendering/mvp_manager.h"
#include "renderer/rendering/renderer_manager.h"
#include "renderer/opengl/shader_manager.h"


// Tracks number of running buffer_load_chunk threads
std::atomic<int> buffer_load_chunk_thread_counter = 0;
// Current layer renderer is on
unsigned int current_layer;

/**
 * Places tiles from specified chunk into its position within the provided buffer
 * @param buffer Holds positions for the textures
 * @param max_buffer_span Span of the buffer
 * @param max_buffer_height Height of the buffer
 * @param renderer Renderer which containing the spritemap positions for the tiles which will be drawn
 * @param tiles Chunk tiles to render
 * @param buffer_x Beginning position of the chunk in buffer (tiles)
 * @param buffer_y Beginning position of the chunk in buffer (tiles)
 * @param func Function which returns internal name of sprite given a Chunk_tile <br>
 * return empty string to render transparency
 * 
 * @exception AccessViolations will occur if input buffer_x/y is greater than span/height
 */
void buffer_load_chunk(float* buffer, 
                       const unsigned int max_buffer_span, const unsigned int max_buffer_height,
                       const jactorio::renderer::Renderer* renderer,
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
			const std::string& sprite_name = func(tiles[tile_y * 32 + tile_x]);

			const unsigned int buffer_offset = 
				((buffer_y + tile_y) * max_buffer_span + (buffer_x + tile_x)) * 8;

			if (sprite_name.empty()) {
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
				const auto positions = renderer->get_sprite_spritemap_coords(sprite_name);
				
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

void jactorio::renderer::world_renderer::draw_chunks(const Renderer& renderer,
                                                     const int window_start_x,
                                                     const int window_start_y,
                                                     const int chunk_start_x,
                                                     const int chunk_start_y,
                                                     const unsigned int chunk_amount_x,
                                                     const unsigned int chunk_amount_y,
                                                     const get_tile_prototype_func& func) {
	// Render chunks
	const unsigned short buffer_span = renderer.get_grid_size_x();
	const unsigned short buffer_height = renderer.get_grid_size_y();

	auto* buffer = renderer.get_texture_grid_buffer();

	std::vector<std::future<void>> chunk_load_threads;
	chunk_load_threads.reserve(static_cast<unsigned long long>(chunk_amount_x) * chunk_amount_y);
	
	for (int chunk_y = 0; chunk_y < chunk_amount_y; ++chunk_y) {
		const int chunk_y_offset = chunk_y * 32 + window_start_y;

		for (int chunk_x = 0; chunk_x < chunk_amount_x; ++chunk_x) {
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
			++buffer_load_chunk_thread_counter;
			
			chunk_load_threads.emplace_back(
				std::async(std::launch::async,
				           buffer_load_chunk,
				           buffer, buffer_span, buffer_height,
				           &renderer, tiles,
				           chunk_x_offset, chunk_y_offset,
				           func)
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

	
	// Rendering layers
	for (unsigned int layer_index = 0; layer_index < renderer_manager::prototype_layer_count; ++layer_index) {
		current_layer = layer_index;

		// // Create layer number, e.g layer 0
		// std::string s("Layer ");
		// s.append(std::to_string(layer_index));
		// auto layer_rendering_timer = core::Execution_timer(s);

		const auto get_tile_proto_func = [](const game::Chunk_tile& chunk_tile) {
			const auto& protos = chunk_tile.tile_prototypes;

			if (current_layer >= protos.size())
				return std::string{};

			return protos[current_layer]->name;
		};

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
		            get_tile_proto_func
		);
		renderer->draw(glm::vec3(0, 0, 0));
	}
	// Resources
	{
		const auto get_tile_proto_func = [](const game::Chunk_tile& chunk_tile) {
			if (chunk_tile.resource_prototype != nullptr)
				return chunk_tile.resource_prototype->name;

			return std::string{};
		};
		
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
		            get_tile_proto_func
		);
		
		renderer->draw(glm::vec3(0, 0, 0));
	}
}
