// 
// mining_drill.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/04/2020
// Last modified: 04/04/2020
// 

#include "data/prototype/entity/mining_drill.h"


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

std::pair<uint16_t, uint16_t> jactorio::data::Mining_drill::map_placement_orientation(placementOrientation orientation,
                                                                                      game::World_data& world_data,
                                                                                      std::pair<int, int> world_coords) const {
	return {8, 0};
}

// ======================================================================

void jactorio::data::Mining_drill::on_defer_time_elapsed(Unique_data_base* unique_data) {

}

void jactorio::data::Mining_drill::on_build(game::World_data& world_data,
                                            std::pair<game::World_data::world_coord, game::World_data::world_coord> world_coords,
                                            game::Chunk_tile_layer& tile_layer,
                                            const uint16_t frame, const placementOrientation orientation) const {
	auto* drill_data = new Mining_drill_data();
	tile_layer.unique_data = drill_data;

	drill_data->set = 8;
	drill_data->frame = frame;
}
