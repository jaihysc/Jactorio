#include "renderer/rendering/world_renderer.h"

#include <future>
#include <game/logic/transport_line_structure.h>
#include <game/logic/transport_line_controller.h>

#include "core/debug/execution_timer.h"

#include "game/world/world_manager.h"
#include "game/world/world_generator.h"

#include "renderer/rendering/mvp_manager.h"
#include "renderer/opengl/shader_manager.h"

using tile_draw_func = unsigned int (*)(const jactorio::game::Chunk_tile&);
using object_draw_func = unsigned int (*)(const jactorio::game::Chunk_object_layer&);

// Functions for drawing each layer, they are accessed by layer_index
tile_draw_func tile_layer_get_sprite_id_func[]{
	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto x = chunk_tile.get_layer_tile_prototype(jactorio::game::Chunk_tile::chunk_layer::base);
		return x->sprite->internal_id;
	},

	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto y = chunk_tile.get_layer_entity_prototype(jactorio::game::Chunk_tile::chunk_layer::resource);
		// Sprites are guaranteed not nullptr
		if (y == nullptr)
			return 0u;
		return y->sprite->internal_id;
	},
	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto y = chunk_tile.get_layer_entity_prototype(jactorio::game::Chunk_tile::chunk_layer::entity);
		if (y == nullptr)
			return 0u;
		return y->sprite->internal_id;
	},

	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto z = chunk_tile.get_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::overlay);
		if (z == nullptr)
			return 0u;
		return z->internal_id;
	}
};

object_draw_func object_layer_get_sprite_id_func[]{
	// Trees?!
	[](const jactorio::game::Chunk_object_layer& layer) {
		auto* entity = static_cast<jactorio::data::Entity*>(layer.prototype_data);
		if (entity == nullptr || entity->sprite == nullptr)
			return 0u;
		return entity->sprite->internal_id;
	},
	// Debug overlay
	[](const jactorio::game::Chunk_object_layer& layer) {
		auto* sprite = static_cast<jactorio::data::Sprite*>(layer.prototype_data);
		return sprite->internal_id;
	},
};


// prepare_chunk_draw_data will select either
// prepare_tile_data or prepare_object_data based on the layer it is rendering
void prepare_tile_data(const unsigned layer_index,
					   jactorio::renderer::Renderer_layer* layer,
					   const float chunk_y_offset, const float chunk_x_offset,
					   jactorio::game::Chunk* const chunk) {
	// Load chunk into buffer
	jactorio::game::Chunk_tile* tiles = chunk->tiles_ptr();


	// Iterate through and load tiles of a chunk into layer for rendering
	for (unsigned int tile_y = 0; tile_y < 32; ++tile_y) {
		const float uv_y = (chunk_y_offset + tile_y) * jactorio::renderer::Renderer::tile_width;

		for (unsigned int tile_x = 0; tile_x < 32; ++tile_x) {
			const auto& tile = tiles[tile_y * 32 + tile_x];
			const unsigned int internal_id = tile_layer_get_sprite_id_func[layer_index](tile);

			// Internal id of 0 indicates no tile
			if (internal_id == 0) {
				continue;
			}

			auto positions = jactorio::renderer::Renderer::get_spritemap_coords(internal_id);

			// Calculate the correct UV coordinates for multi-tile entities
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
				positions.bottom_right.x =
					positions.bottom_right.x - len_x * static_cast<float>(layer_tile.multi_tile_span -
																		  x_multiplier - 1);
				positions.bottom_right.y = positions.bottom_right.y - len_y * y_multiplier;

				// top left
				positions.top_left.x = positions.top_left.x + len_x * x_multiplier;
				positions.top_left.y =
					positions.top_left.y + len_y * static_cast<float>(layer_tile.multi_tile_height -
																	  y_multiplier - 1);
			}

			// Calculate screen coordinates
			const float uv_x = (chunk_x_offset + tile_x) * jactorio::renderer::Renderer::tile_width;

			layer->push_back(
				jactorio::renderer::Renderer_layer::Element(
					{
						{
							uv_x,
							uv_y
						},
						// One tile right and down
						{
							uv_x + static_cast<float>(jactorio::renderer::Renderer::tile_width),
							uv_y + static_cast<float>(jactorio::renderer::Renderer::tile_width)
						}
					},
					{positions.top_left, positions.bottom_right}
				)
			);

		}
	}
}

void prepare_transport_segment_data(jactorio::renderer::Renderer_layer* layer,
									const float chunk_y_offset, const float chunk_x_offset,
									const jactorio::game::Transport_line_segment* line_segment,
									std::deque<jactorio::game::transport_line_item>& line_segment_side,
									float& offset_x, float& offset_y) {
	using namespace jactorio::game;

	// TODO give these constants
	switch (line_segment->direction) {
		case jactorio::game::Transport_line_segment::move_dir::up:
			offset_x += transport_line_c::line_up_r_item_offset;
			break;
		case jactorio::game::Transport_line_segment::move_dir::right:
			offset_y += transport_line_c::line_right_r_item_offset;
			break;
		case jactorio::game::Transport_line_segment::move_dir::down:
			offset_x += transport_line_c::line_down_r_item_offset;
			break;
		case jactorio::game::Transport_line_segment::move_dir::left:
			offset_y += transport_line_c::line_left_r_item_offset;
			break;
	}

	for (const auto& line_item : line_segment_side) {
		// Think of a constant expression for this to avoid having to switch
		switch (line_segment->direction) {
			case jactorio::game::Transport_line_segment::move_dir::up:
				offset_y += line_item.first;
				break;
			case jactorio::game::Transport_line_segment::move_dir::right:
				offset_x -= line_item.first;
				break;
			case jactorio::game::Transport_line_segment::move_dir::down:
				offset_y -= line_item.first;
				break;
			case jactorio::game::Transport_line_segment::move_dir::left:
				offset_x += line_item.first;
				break;
		}

		const auto& uv_pos = jactorio::renderer::Renderer::get_spritemap_coords(line_item.second->sprite->internal_id);

		float top_x = (chunk_x_offset + offset_x)
					  * static_cast<float>(jactorio::renderer::Renderer::tile_width);
		float top_y = (chunk_y_offset + offset_y)
					  * static_cast<float>(jactorio::renderer::Renderer::tile_width);

		layer->push_back(jactorio::renderer::Renderer_layer::Element(
			{
				{
					{top_x, top_y},
					{
						top_x
						+ jactorio::game::transport_line_c::item_width *
						  static_cast<float>(jactorio::renderer::Renderer::tile_width),
						top_y
						+ jactorio::game::transport_line_c::item_width *
						  static_cast<float>(jactorio::renderer::Renderer::tile_width)
					},
				},
				{   uv_pos.top_left, uv_pos.bottom_right}
			}
		));
	}
}


void prepare_object_data(const unsigned layer_index,
						 jactorio::renderer::Renderer_layer* layer,
						 const float chunk_y_offset, const float chunk_x_offset,
						 jactorio::game::Chunk* const chunk) {
	// Draw logic chunk contents if it exists
	auto& logic_chunks = jactorio::game::world_manager::logic_get_all_chunks();
	auto logic_chunk = logic_chunks.find(chunk);

	if (logic_chunk != logic_chunks.end()) {
		auto& transport_line_layer = logic_chunk->second.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);
		for (const auto& line_layer : transport_line_layer) {
			auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			float offset_x = line_layer.position_x;
			float offset_y = line_layer.position_y;

			// TODO better calculate the offset to represent the position of the rail
			// This is probably due to how the endings work and that the offset begins from top left, but I am not too sure
			switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::move_dir::up:
					offset_y += 0.7f;
					break;
//				case jactorio::game::Transport_line_segment::move_dir::right:
//					offset_x += 0.7f;
//					break;
//				case jactorio::game::Transport_line_segment::move_dir::down:
//					offset_y += 0.7f;
//					break;
				case jactorio::game::Transport_line_segment::move_dir::left:
					offset_x += 0.7f;
					break;
			}

			// Left TODO
//			prepare_transport_segment_data(layer,
//										   chunk_y_offset, chunk_x_offset,
//										   line_segment, line_segment->left, offset_x, offset_y);

			prepare_transport_segment_data(layer,
										   chunk_y_offset, chunk_x_offset,
										   line_segment, line_segment->right, offset_x, offset_y);
		}
	}

	auto& objects = chunk->objects[layer_index];
	for (auto& object_layer : objects) {
		const unsigned int internal_id = object_layer_get_sprite_id_func[layer_index](object_layer);

		// Internal id of 0 indicates no tile
		if (internal_id == 0)
			continue;

		const auto& uv_pos = jactorio::renderer::Renderer::get_spritemap_coords(internal_id);

		layer->push_back(jactorio::renderer::Renderer_layer::Element(
			{
				{
					(chunk_x_offset + object_layer.position_x)
					* static_cast<float>(jactorio::renderer::Renderer::tile_width),

					(chunk_y_offset + object_layer.position_y)
					* static_cast<float>(jactorio::renderer::Renderer::tile_width)
				},
				{
					(chunk_x_offset + object_layer.position_x + object_layer.size_x)
					* static_cast<float>(jactorio::renderer::Renderer::tile_width),

					(chunk_y_offset + object_layer.position_y + object_layer.size_y)
					* static_cast<float>(jactorio::renderer::Renderer::tile_width)
				}
			},
			{uv_pos.top_left, uv_pos.bottom_right}
		));
	}
}

void jactorio::renderer::world_renderer::prepare_chunk_draw_data(const int layer_index, const bool is_tile_layer,
																 const int render_offset_x, const int render_offset_y,
																 const int chunk_start_x, const int chunk_start_y,
																 const int chunk_amount_x, const int chunk_amount_y,
																 Renderer_layer* layer) {
	void (* prepare_func)(unsigned, Renderer_layer*, float, float, game::Chunk*);
	if (is_tile_layer)  // Either prepare tiles or objects in chunk
		prepare_func = &prepare_tile_data;
	else
		prepare_func = &prepare_object_data;

	for (int chunk_y = 0; chunk_y < chunk_amount_y; ++chunk_y) {
		const int chunk_y_offset = chunk_y * 32 + render_offset_y;

		for (int chunk_x = 0; chunk_x < chunk_amount_x; ++chunk_x) {
			const int chunk_x_offset = chunk_x * 32 + render_offset_x;

			auto* chunk = game::world_manager::get_chunk(chunk_start_x + chunk_x,
														 chunk_start_y + chunk_y);

			// Generate chunk if non existent
			if (chunk == nullptr) {
				game::world_generator::queue_chunk_generation(
					chunk_start_x + chunk_x,
					chunk_start_y + chunk_y);
				continue;
			}

			prepare_func(layer_index, layer,
						 static_cast<float>(chunk_y_offset), static_cast<float>(chunk_x_offset),
						 chunk);
		}
	}
}

void jactorio::renderer::world_renderer::prepare_logic_chunk_draw_data(jactorio::game::Logic_chunk* l_chunk,
																	   jactorio::renderer::Renderer_layer* layer) {
}


void jactorio::renderer::world_renderer::render_player_position(Renderer* renderer,
																const float player_x,
																const float player_y) {
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

	// -64 to hide the 2 extra chunk around the outside screen
	const auto window_start_x = tile_start_x - 64;
	const auto window_start_y = tile_start_y - 64;

	// Match the tile offset with start offset
	chunk_start_x -= 2;
	chunk_start_y -= 2;

	// Calculate the maximum number of chunks which can be rendered
	const auto amount_x = (renderer->get_grid_size_x() - window_start_x) / 32 + 1;  // Render 1 extra chunk on the edge
	const auto amount_y = (renderer->get_grid_size_y() - window_start_y) / 32 + 1;


	auto* layer_1 = &renderer->render_layer;
	auto* layer_2 = &renderer->render_layer2;
	// !Very important! Remember to clear the layers or else it will keep trying to append into it
	layer_2->clear();

	std::future<void> preparing_thread1;
	std::future<void> preparing_thread2 =
		std::async(std::launch::async, prepare_chunk_draw_data,
				   0, true,
				   window_start_x, window_start_y,
				   chunk_start_x, chunk_start_y,
				   amount_x, amount_y,
				   layer_2);

	bool using_buffer1 = true;
	// Begin at index 1, since index 0 is handled above
	for (unsigned int layer_index = 1; layer_index < game::Chunk_tile::tile_layer_count; ++layer_index) {
		// Prepare 1
		if (using_buffer1) {
			layer_1->clear();
			preparing_thread1 =
				std::async(std::launch::async, prepare_chunk_draw_data,
						   layer_index, true,
						   window_start_x, window_start_y,
						   chunk_start_x, chunk_start_y,
						   amount_x, amount_y,
						   layer_1);

			preparing_thread2.wait();

			renderer->render_layer2.g_update_data();
			renderer->render_layer2.g_buffer_bind();
			Renderer::g_draw(layer_2->get_element_count());
		}
			// Prepare 2
		else {
			layer_2->clear();
			preparing_thread2 =
				std::async(std::launch::async, prepare_chunk_draw_data,
						   layer_index, true,
						   window_start_x, window_start_y,
						   chunk_start_x, chunk_start_y,
						   amount_x, amount_y,
						   layer_2);

			preparing_thread1.wait();

			renderer->render_layer.g_update_data();
			renderer->render_layer.g_buffer_bind();
			Renderer::g_draw(layer_1->get_element_count());
		}
		using_buffer1 = !using_buffer1;
	}

	// ==============================================================
	// Draw object layers
	for (unsigned int layer_index = 0;
		 layer_index < game::Chunk::object_layer_count; ++layer_index) {
		// Prepare 1
		if (using_buffer1) {
			layer_1->clear();
			preparing_thread1 =
				std::async(std::launch::async, prepare_chunk_draw_data,
						   layer_index, false,
						   window_start_x, window_start_y,
						   chunk_start_x, chunk_start_y,
						   amount_x, amount_y,
						   layer_1);

			preparing_thread2.wait();

			renderer->render_layer2.g_update_data();
			renderer->render_layer2.g_buffer_bind();
			Renderer::g_draw(layer_2->get_element_count());
		}
			// Prepare 2
		else {
			layer_2->clear();
			preparing_thread2 =
				std::async(std::launch::async, prepare_chunk_draw_data,
						   layer_index, false,
						   window_start_x, window_start_y,
						   chunk_start_x, chunk_start_y,
						   amount_x, amount_y,
						   layer_2);

			preparing_thread1.wait();

			renderer->render_layer.g_update_data();
			renderer->render_layer.g_buffer_bind();
			Renderer::g_draw(layer_1->get_element_count());
		}
		using_buffer1 = !using_buffer1;
	}

	// Wait for the final layer to draw
	if (using_buffer1) {
		preparing_thread2.wait();
		layer_2->g_update_data();
		layer_2->g_buffer_bind();
		Renderer::g_draw(layer_2->get_element_count());
	}
	else {
		preparing_thread1.wait();
		layer_1->g_update_data();
		layer_1->g_buffer_bind();
		Renderer::g_draw(layer_1->get_element_count());
	}
}
