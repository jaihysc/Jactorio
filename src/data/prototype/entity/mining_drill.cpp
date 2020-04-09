// 
// mining_drill.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/04/2020
// Last modified: 04/09/2020
// 

#include "data/prototype/entity/mining_drill.h"

#include "data/data_manager.h"
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

void jactorio::data::Mining_drill::on_defer_time_elapsed(game::Deferral_timer& timer, Unique_data_base* unique_data) const {
	// TODO this is temporary
	auto* drill_data = static_cast<Mining_drill_data*>(unique_data);

	const std::string iname = "__base__/coal-item";
	auto* item = data::data_manager::data_raw_get<Item>(dataCategory::item, iname);

	drill_data->item_output.insert({item, 1});
	register_mine_callback(timer, drill_data);
}


bool jactorio::data::Mining_drill::on_can_build(const game::World_data& world_data,
                                                std::pair<game::World_data::world_coord, game::World_data::world_coord>
                                                world_coords) {
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
	// TODO temporary
	auto& layer = world_data.get_tile_world_coords(world_coords.first, world_coords.second - 1)
	                        ->get_layer(game::Chunk_tile::chunkLayer::entity);

	auto* drill_data =
		new Mining_drill_data(*layer.unique_data,
		                      game::item_logistics::can_accept_item(world_data,
		                                                            world_coords.first,
		                                                            world_coords.second - 1),
		                      placementOrientation::down
		);

	tile_layer.unique_data = drill_data;

	drill_data->set = map_placement_orientation(orientation, world_data, world_coords).first;
	drill_data->frame = frame;

	register_mine_callback(world_data.deferral_timer, drill_data);
}

void jactorio::data::Mining_drill::on_neighbor_update(const game::World_data& world_data,
                                                      std::pair<game::World_data::world_coord, game::World_data::world_coord>
                                                      world_coords,
                                                      placementOrientation orientation) const {
}
