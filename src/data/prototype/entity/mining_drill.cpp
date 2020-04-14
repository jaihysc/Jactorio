// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/04/2020

#include "data/prototype/entity/mining_drill.h"

#include "data/data_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "game/logic/item_logistics.h"


jactorio::data::Sprite* jactorio::data::Mining_drill::on_r_get_sprite(Unique_data_base* unique_data) const {
	const auto set = static_cast<Renderable_data*>(unique_data)->set;

	if (set <= 7)
		return this->sprite;

	if (set <= 15)
		return this->sprite_e;

	if (set <= 23)
		return this->sprite_s;

	return this->sprite_w;
}

std::pair<uint16_t, uint16_t> jactorio::data::Mining_drill::map_placement_orientation(const placementOrientation orientation,
                                                                                      game::World_data& world_data,
                                                                                      std::pair<int, int> world_coords) const {
	switch (orientation) {
	case placementOrientation::up:
		return {0, 0};
	case placementOrientation::right:
		return {8, 0};
	case placementOrientation::down:
		return {16, 0};
	case placementOrientation::left:
		return {24, 0};

	default:
		assert(false);  // Missing switch case
		return {0, 0};
	}
}

// ======================================================================

void jactorio::data::Mining_drill::register_mine_callback(game::Deferral_timer& timer, Mining_drill_data* unique_data) const {
	// TODO
	timer.register_from_tick(*this, unique_data, 100);
}

jactorio::data::Item* jactorio::data::Mining_drill::find_output_item(const game::World_data& world_data,
                                                                     game::World_data::world_pair world_pair) const {
	world_pair.first -= this->mining_radius;
	world_pair.second -= this->mining_radius;

	for (int y = 0; y < 2 * this->mining_radius + this->tile_height; ++y) {
		for (int x = 0; x < 2 * this->mining_radius + this->tile_width; ++x) {
			game::Chunk_tile* tile =
				world_data.get_tile_world_coords(world_pair.first + x, world_pair.second + y);

			game::Chunk_tile_layer& resource = tile->get_layer(game::Chunk_tile::chunkLayer::resource);
			if (resource.prototype_data != nullptr)
				return static_cast<const Resource_entity*>(resource.prototype_data)->get_item();
		}
	}

	return nullptr;
}

void jactorio::data::Mining_drill::on_defer_time_elapsed(game::Deferral_timer& timer, Unique_data_base* unique_data) const {
	// Re-register callback and insert item
	auto* drill_data = static_cast<Mining_drill_data*>(unique_data);

	drill_data->output_tile->insert({drill_data->output_item, 1});
	register_mine_callback(timer, drill_data);
}


bool jactorio::data::Mining_drill::on_can_build(const game::World_data& world_data,
                                                std::pair<game::World_data::world_coord, game::World_data::world_coord>
                                                world_coords) const {
	/*
	 * [ ] [ ] [ ] [ ] [ ]
	 * [ ] [X] [x] [x] [ ]
	 * [ ] [x] [x] [x] [ ]
	 * [ ] [x] [x] [x] [ ]
	 * [ ] [ ] [ ] [ ] [ ]
	 */
	world_coords.first -= this->mining_radius;
	world_coords.second -= this->mining_radius;

	for (int y = 0; y < 2 * this->mining_radius + this->tile_height; ++y) {
		for (int x = 0; x < 2 * this->mining_radius + this->tile_width; ++x) {
			game::Chunk_tile* tile =
				world_data.get_tile_world_coords(world_coords.first + x, world_coords.second + y);

			if (tile->get_layer(game::Chunk_tile::chunkLayer::resource).prototype_data != nullptr)
				return true;
		}
	}

	return false;
}

void jactorio::data::Mining_drill::on_build(game::World_data& world_data,
                                            const std::pair<game::World_data::world_coord, game::World_data::world_coord>
                                            world_coords,
                                            game::Chunk_tile_layer& tile_layer,
                                            const uint16_t frame, const placementOrientation orientation) const {

	tile_layer.unique_data = new Mining_drill_data();
	auto* drill_data = static_cast<Mining_drill_data*>(tile_layer.unique_data);

	drill_data->output_item = find_output_item(world_data, world_coords);
	assert(drill_data->output_item != nullptr);  // Should not have been allowed to be placed on no resources

	drill_data->set = map_placement_orientation(orientation, world_data, world_coords).first;
	drill_data->frame = frame;

	game::World_data::world_pair output_coords = this->resource_output.get(orientation);
	output_coords.first += world_coords.first;
	output_coords.second += world_coords.second;
	on_neighbor_update(world_data, output_coords, world_coords, orientation);
}

void jactorio::data::Mining_drill::on_neighbor_update(game::World_data& world_data,
                                                      const game::World_data::world_pair emit_world_coords,
                                                      const game::World_data::world_pair receive_world_coords,
                                                      placementOrientation emit_orientation) const {
	const game::Item_insert_destination::insert_func output_item_func =
		game::item_logistics::can_accept_item(world_data,
		                                      emit_world_coords.first,
		                                      emit_world_coords.second);

	// Do not register callback to mine items if there is no valid entity to output items to
	if (output_item_func) {
		Mining_drill_data* drill_data;
		{
			auto& self_layer = world_data.get_tile_world_coords(receive_world_coords.first,
			                                                    receive_world_coords.second)
			                             ->get_layer(game::Chunk_tile::chunkLayer::entity);
			// Use the top left tile
			if (self_layer.is_multi_tile())
				drill_data = static_cast<Mining_drill_data*>(self_layer.get_multi_tile_top_left()->unique_data);
			else
				drill_data = static_cast<Mining_drill_data*>(self_layer.unique_data);
		}

		auto& output_layer = world_data.get_tile_world_coords(emit_world_coords.first,
		                                                      emit_world_coords.second)
		                               ->get_layer(game::Chunk_tile::chunkLayer::entity);

		drill_data->output_tile.emplace(*output_layer.unique_data, output_item_func, emit_orientation);
		register_mine_callback(world_data.deferral_timer, drill_data);
	}
}
