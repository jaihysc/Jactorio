// 
// world_renderer.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 11/15/2019
// Last modified: 03/19/2020
// 

#include "renderer/rendering/world_renderer.h"

#include <future>
#include <game/logic/transport_line_structure.h>
#include <game/logic/transport_line_controller.h>

#include "core/debug/execution_timer.h"

#include "game/world/world_data.h"
#include "game/world/chunk_tile_getters.h"

#include "renderer/rendering/mvp_manager.h"
#include "renderer/opengl/shader_manager.h"

using tile_draw_func = jactorio::core::Quad_position (*)(const jactorio::game::Chunk_tile&);
using object_draw_func = unsigned int (*)(const jactorio::game::Chunk_object_layer&);

const jactorio::core::Quad_position no_draw{{-1.f, -1.f}, {-1.f, -1.f}};

void apply_uv_offset(jactorio::core::Quad_position& uv, const jactorio::core::Quad_position& uv_offset) {
	const auto difference = uv.bottom_right - uv.top_left;

	assert(difference.x >= 0);
	assert(difference.y >= 0);

	// Calculate bottom first since it needs the unmodified top_left
	uv.bottom_right = uv.top_left + difference * uv_offset.bottom_right;
	uv.top_left += difference * uv_offset.top_left;
}


/// \brief Functions for drawing each layer, they are accessed by layer_index
/// \remark return top_left.x -1 to skip
tile_draw_func tile_layer_get_sprite_id_func[]{
	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto t =
			jactorio::game::chunk_tile_getter::get_tile_prototype(
				chunk_tile, jactorio::game::Chunk_tile::chunkLayer::base);

		auto* unique_data =
			static_cast<jactorio::data::Renderable_data*>(
				chunk_tile.get_layer(jactorio::game::Chunk_tile::chunkLayer::base).unique_data);

		auto uv = jactorio::renderer::Renderer::get_spritemap_coords(t->on_r_get_sprite(unique_data)->internal_id);

		if (unique_data)
			apply_uv_offset(uv, t->sprite->get_coords_trimmed(unique_data->set, unique_data->frame));

		return uv;
	},

	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto t =
			jactorio::game::chunk_tile_getter::get_entity_prototype(
				chunk_tile, jactorio::game::Chunk_tile::chunkLayer::resource);
		// Sprites are guaranteed not nullptr
		if (t == nullptr)
			return no_draw;

		auto* unique_data =
			static_cast<jactorio::data::Renderable_data*>(
				chunk_tile.get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).unique_data);

		auto uv = jactorio::renderer::Renderer::get_spritemap_coords(t->on_r_get_sprite(unique_data)->internal_id);


		if (unique_data)  // Unique data may not be initialized by the time this is drawn due to concurrency
			apply_uv_offset(uv, t->sprite->get_coords_trimmed(unique_data->set, unique_data->frame));

		return uv;
	},
	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto t =
			jactorio::game::chunk_tile_getter::get_entity_prototype(
				chunk_tile, jactorio::game::Chunk_tile::chunkLayer::entity);
		if (t == nullptr)
			return no_draw;

		auto* unique_data =
			static_cast<jactorio::data::Renderable_data*>(
				chunk_tile.get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data);

		auto uv = jactorio::renderer::Renderer::get_spritemap_coords(t->on_r_get_sprite(unique_data)->internal_id);

		if (unique_data)  // Unique data may not be initialized by the time this is drawn due to concurrency
			apply_uv_offset(uv, t->sprite->get_coords_trimmed(unique_data->set, unique_data->frame));

		return uv;
	},

	[](const jactorio::game::Chunk_tile& chunk_tile) {
		const auto t =
			jactorio::game::chunk_tile_getter::get_sprite_prototype(
				chunk_tile, jactorio::game::Chunk_tile::chunkLayer::overlay);
		if (t == nullptr)
			return no_draw;

		auto* unique_data =
			static_cast<jactorio::data::Renderable_data*>(
				chunk_tile.get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay).unique_data);

		auto uv = jactorio::renderer::Renderer::get_spritemap_coords(t->internal_id);

		if (unique_data)  // Unique data may not be initialized by the time this is drawn due to concurrency
			apply_uv_offset(uv, t->get_coords_trimmed(unique_data->set, unique_data->frame));

		return uv;
	}
};

object_draw_func object_layer_get_sprite_id_func[]{
	// Trees?!
	[](const jactorio::game::Chunk_object_layer& layer) {
		auto* entity = static_cast<jactorio::data::Entity*>(layer.prototype_data);
		if (entity == nullptr || entity->sprite == nullptr)
			return 0u;
		return entity->on_r_get_sprite(layer.unique_data)->internal_id;
	},
	// Debug overlay
	[](const jactorio::game::Chunk_object_layer& layer) {
		auto* sprite = static_cast<jactorio::data::Sprite*>(layer.prototype_data);
		return sprite->internal_id;
	},
};


// prepare_chunk_draw_data will select either
// prepare_tile_data or prepare_object_data based on the layer it is rendering
void prepare_tile_data(jactorio::game::World_data&,
                       const unsigned layer_index,
                       jactorio::renderer::Renderer_layer* layer,
                       const float chunk_y_offset, const float chunk_x_offset,
                       jactorio::game::Chunk* const chunk) {
	// Load chunk into buffer
	jactorio::game::Chunk_tile* tiles = chunk->tiles_ptr();


	// Iterate through and load tiles of a chunk into layer for rendering
	for (unsigned int tile_y = 0; tile_y < 32; ++tile_y) {
		const float y = (chunk_y_offset + tile_y) * jactorio::renderer::Renderer::tile_width;

		for (unsigned int tile_x = 0; tile_x < 32; ++tile_x) {
			const auto& tile = tiles[tile_y * 32 + tile_x];
			auto uv = tile_layer_get_sprite_id_func[layer_index](tile);

			// uv top left.x = -1.f means no tile
			if (uv.top_left.x == -1.f) {
				continue;
			}

			// Calculate the correct UV coordinates for multi-tile entities
			// Split the sprite into sections and stretch over multiple tiles if this entity is multi tile
			auto& layer_tile = tile.get_layer(layer_index);
			if (layer_tile.multi_tile_span != 1) {
				const auto len_x =
					(uv.bottom_right.x - uv.top_left.x) / static_cast<float>(layer_tile.
						multi_tile_span);

				const auto len_y =
					(uv.bottom_right.y - uv.top_left.y) / static_cast<float>(layer_tile.
						multi_tile_height);

				const auto x_multiplier =
					static_cast<int>(layer_tile.multi_tile_index) % layer_tile.multi_tile_span;
				const auto y_multiplier =
					static_cast<int>(layer_tile.multi_tile_index) / layer_tile.
					multi_tile_span;  // This is correct, divide by width for both calculations

				// Opengl flips vertically, thus the y multiplier is inverted

				// bottom right
				uv.bottom_right.x =
					uv.bottom_right.x - len_x * static_cast<float>(layer_tile.multi_tile_span -
						x_multiplier - 1);
				uv.bottom_right.y = uv.bottom_right.y - len_y * y_multiplier;

				// top left
				uv.top_left.x = uv.top_left.x + len_x * x_multiplier;
				uv.top_left.y =
					uv.top_left.y + len_y * static_cast<float>(layer_tile.multi_tile_height -
						y_multiplier - 1);
			}

			// Calculate screen coordinates
			const float x = (chunk_x_offset + tile_x) * jactorio::renderer::Renderer::tile_width;

			layer->push_back(
				jactorio::renderer::Renderer_layer::Element(
					{
						{
							x,
							y
						},
						// One tile right and down
						{
							x + static_cast<float>(jactorio::renderer::Renderer::tile_width),
							y + static_cast<float>(jactorio::renderer::Renderer::tile_width)
						}
					},
					{uv.top_left, uv.bottom_right}
				)
			);

		}
	}
}

void prepare_transport_segment_data(jactorio::renderer::Renderer_layer* layer,
                                    const float chunk_y_offset, const float chunk_x_offset,
                                    const jactorio::game::Transport_line_segment* line_segment,
                                    std::deque<jactorio::game::transport_line_item>& line_segment_side,
                                    float offset_x, float offset_y) {
	using namespace jactorio::game;

	// Either offset_x or offset_y which will be INCREASED or DECREASED
	float* target_offset;
	float multiplier = 1;  // Either 1 or -1 to add or subtract

	switch (line_segment->direction) {
	case Transport_line_segment::moveDir::up:
		target_offset = &offset_y;
		break;
	case Transport_line_segment::moveDir::right:
		target_offset = &offset_x;
		multiplier = -1;
		break;
	case Transport_line_segment::moveDir::down:
		target_offset = &offset_y;
		multiplier = -1;
		break;
	case Transport_line_segment::moveDir::left:
		target_offset = &offset_x;
		break;
	}

	for (const auto& line_item : line_segment_side) {
		// Move the target offset (up or down depending on multiplier)
		*target_offset += line_item.first.getAsDouble() * multiplier;

		const auto& uv_pos = jactorio::renderer::Renderer::get_spritemap_coords(line_item.second->sprite->internal_id);

		const float top_x = (chunk_x_offset + offset_x)
			* static_cast<float>(jactorio::renderer::Renderer::tile_width);
		const float top_y = (chunk_y_offset + offset_y)
			* static_cast<float>(jactorio::renderer::Renderer::tile_width);

		layer->push_back(jactorio::renderer::Renderer_layer::Element(
			{
				{
					{top_x, top_y},
					{
						static_cast<float>(top_x
							+ transport_line_c::item_width *
							static_cast<float>(jactorio::renderer::Renderer::tile_width)),

						static_cast<float>(top_y
							+ transport_line_c::item_width *
							static_cast<float>(jactorio::renderer::Renderer::tile_width))
					},
				},
				{uv_pos.top_left, uv_pos.bottom_right}
			}
		));
	}
}


void prepare_object_data(jactorio::game::World_data& world_data,
                         const unsigned layer_index,
                         jactorio::renderer::Renderer_layer* layer,
                         const float chunk_y_offset, const float chunk_x_offset,
                         jactorio::game::Chunk* const chunk) {

	// Draw logic chunk contents if it exists
	auto& logic_chunks = world_data.logic_get_all_chunks();
	auto logic_chunk = logic_chunks.find(chunk);

	if (logic_chunk != logic_chunks.end()) {
		auto& transport_line_layer = logic_chunk->second.get_struct(
			jactorio::game::Logic_chunk::structLayer::transport_line);
		for (const auto& line_layer : transport_line_layer) {
			auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			// Don't render if items are not marked visible! Wow!
			if (!line_segment->item_visible)
				continue;

			float offset_x = line_layer.position_x;
			float offset_y = line_layer.position_y;

			// Left
			// The offsets for straight are always applied to bend left and right
			switch (line_segment->direction) {
			case jactorio::game::Transport_line_segment::moveDir::up:
				offset_x += jactorio::game::transport_line_c::line_up_l_item_offset_x;
				break;
			case jactorio::game::Transport_line_segment::moveDir::right:
				offset_y += jactorio::game::transport_line_c::line_right_l_item_offset_y;
				break;
			case jactorio::game::Transport_line_segment::moveDir::down:
				offset_x += jactorio::game::transport_line_c::line_down_l_item_offset_x;
				break;
			case jactorio::game::Transport_line_segment::moveDir::left:
				offset_y += jactorio::game::transport_line_c::line_left_l_item_offset_y;
				break;
			}

			// Left side
			switch (line_segment->termination_type) {
			case jactorio::game::Transport_line_segment::terminationType::straight:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y -= jactorio::game::transport_line_c::line_left_up_straight_item_offset;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_down_straight_item_offset;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_right_down_straight_item_offset;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x -= jactorio::game::transport_line_c::line_left_up_straight_item_offset;
					break;
				}
				break;

			case jactorio::game::Transport_line_segment::terminationType::bend_left:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y += jactorio::game::transport_line_c::line_up_bl_l_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_bl_l_item_offset_x;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_down_bl_l_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x += jactorio::game::transport_line_c::line_left_bl_l_item_offset_x;
					break;
				}
				break;

			case jactorio::game::Transport_line_segment::terminationType::bend_right:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y += jactorio::game::transport_line_c::line_up_br_l_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_br_l_item_offset_x;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_down_br_l_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x += jactorio::game::transport_line_c::line_left_br_l_item_offset_x;
					break;
				}
				break;

				// Side insertion
			case jactorio::game::Transport_line_segment::terminationType::right_only:
			case jactorio::game::Transport_line_segment::terminationType::left_only:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y += jactorio::game::transport_line_c::line_up_single_side_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_single_side_item_offset_x;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_down_single_side_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x += jactorio::game::transport_line_c::line_left_single_side_item_offset_x;
					break;
				}
				break;
			}
			prepare_transport_segment_data(layer,
			                               chunk_y_offset, chunk_x_offset,
			                               line_segment, line_segment->left, offset_x, offset_y);

			// Right
			offset_x = line_layer.position_x;
			offset_y = line_layer.position_y;

			// The offsets for straight are always applied to bend left and right
			switch (line_segment->direction) {
			case jactorio::game::Transport_line_segment::moveDir::up:
				offset_x += jactorio::game::transport_line_c::line_up_r_item_offset_x;
				break;
			case jactorio::game::Transport_line_segment::moveDir::right:
				offset_y += jactorio::game::transport_line_c::line_right_r_item_offset_y;
				break;
			case jactorio::game::Transport_line_segment::moveDir::down:
				offset_x += jactorio::game::transport_line_c::line_down_r_item_offset_x;
				break;
			case jactorio::game::Transport_line_segment::moveDir::left:
				offset_y += jactorio::game::transport_line_c::line_left_r_item_offset_y;
				break;
			}


			// Right side
			switch (line_segment->termination_type) {
			case jactorio::game::Transport_line_segment::terminationType::straight:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y -= jactorio::game::transport_line_c::line_left_up_straight_item_offset;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_down_straight_item_offset;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_right_down_straight_item_offset;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x -= jactorio::game::transport_line_c::line_left_up_straight_item_offset;
					break;
				}
				break;

			case jactorio::game::Transport_line_segment::terminationType::bend_left:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y += jactorio::game::transport_line_c::line_up_bl_r_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_bl_r_item_offset_x;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_down_bl_r_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x += jactorio::game::transport_line_c::line_left_bl_r_item_offset_x;
					break;
				}
				break;

			case jactorio::game::Transport_line_segment::terminationType::bend_right:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y += jactorio::game::transport_line_c::line_up_br_r_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_br_r_item_offset_x;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_down_br_r_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x += jactorio::game::transport_line_c::line_left_br_r_item_offset_x;
					break;
				}
				break;

				// Side insertion
			case jactorio::game::Transport_line_segment::terminationType::right_only:
			case jactorio::game::Transport_line_segment::terminationType::left_only:
				switch (line_segment->direction) {
				case jactorio::game::Transport_line_segment::moveDir::up:
					offset_y += jactorio::game::transport_line_c::line_up_single_side_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::right:
					offset_x += jactorio::game::transport_line_c::line_right_single_side_item_offset_x;
					break;
				case jactorio::game::Transport_line_segment::moveDir::down:
					offset_y += jactorio::game::transport_line_c::line_down_single_side_item_offset_y;
					break;
				case jactorio::game::Transport_line_segment::moveDir::left:
					offset_x += jactorio::game::transport_line_c::line_left_single_side_item_offset_x;
					break;
				}
				break;
			}
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

void jactorio::renderer::world_renderer::prepare_chunk_draw_data(game::World_data& world_data,
                                                                 const int layer_index, const bool is_tile_layer,
                                                                 const int render_offset_x, const int render_offset_y,
                                                                 const int chunk_start_x, const int chunk_start_y,
                                                                 const int chunk_amount_x, const int chunk_amount_y,
                                                                 Renderer_layer* layer) {
	void (* prepare_func)(game::World_data&, unsigned, Renderer_layer*, float, float, game::Chunk*);

	if (is_tile_layer)  // Either prepare tiles or objects in chunk
		prepare_func = &prepare_tile_data;
	else
		prepare_func = &prepare_object_data;

	for (int chunk_y = 0; chunk_y < chunk_amount_y; ++chunk_y) {
		const int chunk_y_offset = chunk_y * 32 + render_offset_y;

		for (int chunk_x = 0; chunk_x < chunk_amount_x; ++chunk_x) {
			const int chunk_x_offset = chunk_x * 32 + render_offset_x;

			auto* chunk = world_data.get_chunk(chunk_start_x + chunk_x,
			                                   chunk_start_y + chunk_y);

			// Generate chunk if non existent
			if (chunk == nullptr) {
				world_data.queue_chunk_generation(
					chunk_start_x + chunk_x,
					chunk_start_y + chunk_y);
				continue;
			}

			prepare_func(world_data,
			             layer_index, layer,
			             static_cast<float>(chunk_y_offset), static_cast<float>(chunk_x_offset),
			             chunk);
		}
	}
}

void jactorio::renderer::world_renderer::prepare_logic_chunk_draw_data(game::Logic_chunk* l_chunk,
                                                                       Renderer_layer* layer) {
}


void jactorio::renderer::world_renderer::render_player_position(game::World_data& world_data,
                                                                Renderer* renderer,
                                                                const float player_x, const float player_y) {
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
		           std::ref(world_data), 0, true,
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
				           std::ref(world_data), layer_index, true,
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
				           std::ref(world_data), layer_index, true,
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
				           std::ref(world_data), layer_index, false,
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
				           std::ref(world_data), layer_index, false,
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
