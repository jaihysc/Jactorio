// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/04/2020

#include "data/prototype/entity/mining_drill.h"

#include "data/data_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "game/logic/item_logistics.h"
#include "renderer/gui/gui_menus.h"


void jactorio::data::Mining_drill::on_r_show_gui(game::Player_data& player_data, game::Chunk_tile_layer* tile_layer) const {
	auto* drill_data = static_cast<Mining_drill_data*>(tile_layer->unique_data);

	renderer::gui::mining_drill(player_data, drill_data);
}

std::pair<jactorio::data::Sprite*, jactorio::data::Renderable_data::frame_t> jactorio::data::Mining_drill::on_r_get_sprite(
	Unique_data_base* unique_data, const game_tick_t game_tick) const {
	const auto set = static_cast<Renderable_data*>(unique_data)->set;

	if (set <= 7)
		return {this->sprite, game_tick % this->sprite->frames * this->sprite->sets};

	if (set <= 15)
		return {this->sprite_e, game_tick % this->sprite_e->frames * this->sprite_e->sets};

	if (set <= 23)
		return {this->sprite_s, game_tick % this->sprite_s->frames * this->sprite_s->sets};

	return {this->sprite_w, game_tick % this->sprite_w->frames * this->sprite_w->sets};
}

std::pair<uint16_t, uint16_t> jactorio::data::Mining_drill::map_placement_orientation(const Orientation orientation,
                                                                                      game::World_data&,
                                                                                      const game::World_data::world_pair&) const {
	switch (orientation) {
	case Orientation::up:
		return {0, 0};
	case Orientation::right:
		return {8, 0};
	case Orientation::down:
		return {16, 0};
	case Orientation::left:
		return {24, 0};

	default:
		assert(false);  // Missing switch case
		return {0, 0};
	}
}

// ======================================================================

void jactorio::data::Mining_drill::register_mine_callback(game::Deferral_timer& timer, Mining_drill_data* unique_data) const {
	unique_data->deferral_entry = timer.register_from_tick(*this, unique_data, unique_data->mining_ticks);
}

void jactorio::data::Mining_drill::remove_mine_callback(game::Deferral_timer& timer,
                                                        game::Deferral_timer::deferral_entry& entry) {
	if (entry.second == 0)
		return;

	timer.remove_deferral(entry);
	entry.second = 0;
}

jactorio::data::Item* jactorio::data::Mining_drill::find_output_item(const game::World_data& world_data,
                                                                     game::World_data::world_pair world_pair) const {
	world_pair.first -= this->mining_radius;
	world_pair.second -= this->mining_radius;

	for (int y = 0; y < 2 * this->mining_radius + this->tile_height; ++y) {
		for (int x = 0; x < 2 * this->mining_radius + this->tile_width; ++x) {
			game::Chunk_tile* tile =
				world_data.get_tile(world_pair.first + x, world_pair.second + y);

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
                                                const game::World_data::world_pair& world_coords) const {
	auto coords = world_coords;
	/*
	 * [ ] [ ] [ ] [ ] [ ]
	 * [ ] [X] [x] [x] [ ]
	 * [ ] [x] [x] [x] [ ]
	 * [ ] [x] [x] [x] [ ]
	 * [ ] [ ] [ ] [ ] [ ]
	 */
	coords.first -= this->mining_radius;
	coords.second -= this->mining_radius;

	for (int y = 0; y < 2 * this->mining_radius + this->tile_height; ++y) {
		for (int x = 0; x < 2 * this->mining_radius + this->tile_width; ++x) {
			game::Chunk_tile* tile =
				world_data.get_tile(coords.first + x, coords.second + y);

			if (tile->get_layer(game::Chunk_tile::chunkLayer::resource).prototype_data != nullptr)
				return true;
		}
	}

	return false;
}

void jactorio::data::Mining_drill::on_build(game::World_data& world_data,
                                            const game::World_data::world_pair& world_coords,
                                            game::Chunk_tile_layer& tile_layer,
                                            const Orientation orientation) const {
	tile_layer.unique_data = new Mining_drill_data();
	auto* drill_data       = static_cast<Mining_drill_data*>(tile_layer.unique_data);

	drill_data->output_item = find_output_item(world_data, world_coords);
	assert(drill_data->output_item != nullptr);  // Should not have been allowed to be placed on no resources

	drill_data->set = map_placement_orientation(orientation, world_data, world_coords).first;

	game::World_data::world_pair output_coords = this->resource_output.get(orientation);
	output_coords.first += world_coords.first;
	output_coords.second += world_coords.second;

	drill_data->output_tile_coords = output_coords;

	on_neighbor_update(world_data, output_coords, world_coords, orientation);
}

void jactorio::data::Mining_drill::on_neighbor_update(game::World_data& world_data,
                                                      const game::World_data::world_pair& emit_world_coords,
                                                      const game::World_data::world_pair& receive_world_coords,
                                                      Orientation emit_orientation) const {
	Mining_drill_data* drill_data;
	{
		auto& self_layer = world_data.get_tile(receive_world_coords.first,
		                                       receive_world_coords.second)
		                             ->get_layer(game::Chunk_tile::chunkLayer::entity);
		// Use the top left tile
		if (self_layer.is_multi_tile())
			drill_data = static_cast<Mining_drill_data*>(self_layer.get_multi_tile_top_left()->unique_data);
		else
			drill_data = static_cast<Mining_drill_data*>(self_layer.unique_data);
	}

	// Ignore updates from non output tiles 
	if (emit_world_coords != drill_data->output_tile_coords)
		return;

	const game::Item_insert_destination::insert_func output_item_func =
		game::item_logistics::can_accept_item(world_data,
		                                      emit_world_coords.first,
		                                      emit_world_coords.second);

	// Do not register callback to mine items if there is no valid entity to output items to
	if (output_item_func) {

		auto& output_layer = world_data.get_tile(emit_world_coords.first,
		                                         emit_world_coords.second)
		                               ->get_layer(game::Chunk_tile::chunkLayer::entity);

		drill_data->output_tile.emplace(*output_layer.unique_data, output_item_func, emit_orientation);

		drill_data->mining_ticks =
			static_cast<uint16_t>(static_cast<double>(JC_GAME_HERTZ) * drill_data->output_item->entity_prototype->pickup_time);

		register_mine_callback(world_data.deferral_timer, drill_data);
	}
		// Un-register callback if one is registered
	else if (drill_data->deferral_entry.second != 0) {
		remove_mine_callback(world_data.deferral_timer, drill_data->deferral_entry);
	}
}


void jactorio::data::Mining_drill::on_remove(game::World_data& world_data,
                                             const game::World_data::world_pair /*world_coords*/&,
                                             game::Chunk_tile_layer& tile_layer) const {
	Unique_data_base* drill_data;

	if (tile_layer.is_multi_tile())
		drill_data = tile_layer.get_multi_tile_top_left()->unique_data;
	else
		drill_data = tile_layer.unique_data;

	remove_mine_callback(world_data.deferral_timer, static_cast<Mining_drill_data*>(drill_data)->deferral_entry);
}
