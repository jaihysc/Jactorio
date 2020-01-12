#include "game/logic_loop.h"

#include <chrono>
#include <thread>

#include "jactorio.h"

#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/placement_controller.h"
#include "game/logic/inventory_controller.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"
#include "game/world/world_manager.h"

#include "data/data_manager.h"
#include "data/prototype/item/item.h"
#include "renderer/gui/imgui_manager.h"
#include "data/prototype/tile/resource_tile.h"

bool logic_loop_should_terminate = false;

const float move_speed = 1.1f;

int test_rm_counter = 0;

void jactorio::game::init_logic_loop() {
	// Logic initialization here...
	logic_loop_should_terminate = false;

	// Movement controls
	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed * -1);
	}, GLFW_KEY_W, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed * -1);
	}, GLFW_KEY_W, GLFW_REPEAT);


	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed);
	}, GLFW_KEY_S, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_y(move_speed);
	}, GLFW_KEY_S, GLFW_REPEAT);

	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed * -1);
	}, GLFW_KEY_A, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed * -1);
	}, GLFW_KEY_A, GLFW_REPEAT);

	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed);
	}, GLFW_KEY_D, GLFW_PRESS);
	input_manager::subscribe([]() {
		player_manager::move_player_x(move_speed);
	}, GLFW_KEY_D, GLFW_REPEAT);


	{
		using namespace renderer::imgui_manager;
		// Menus
		input_manager::subscribe([]() {
			set_window_visibility(gui_window::debug, !get_window_visibility(gui_window::debug));
		}, GLFW_KEY_GRAVE_ACCENT, GLFW_RELEASE);
		input_manager::subscribe([]() {
			set_window_visibility(gui_window::character, !get_window_visibility(gui_window::character));
		}, GLFW_KEY_TAB, GLFW_RELEASE);
	}

	// TODO REMOVE | Test data
	Event::subscribe(event_type::game_gui_open, []() {
		data::item_stack* inventory = player_manager::player_inventory;

		using namespace data;
		auto x = data_manager::data_raw_get_all<Item>(data_category::item);
		inventory[0] = std::pair(x[0], 10);
		inventory[1] = std::pair(x[0], 8);
		inventory[4] = std::pair(x[0], 100);
		inventory[5] = std::pair(x[1], 2000);
	});

	// TODO MOVE, test entity placement
	{
		// Place entities
		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured)
				return;

			const data::item_stack* ptr;
			if ((ptr = player_manager::get_selected_item()) != nullptr) {
				const auto tile_selected = mouse_selection::get_mouse_tile_coords();

				// Does an entity already exist at this location?
				if (world_manager::get_tile_world_coords(tile_selected.first, tile_selected.second)
					->get_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::entity) != nullptr)
					return;

				// Entities only
				auto* entity_ptr = static_cast<data::Entity*>(ptr->first->entity_prototype);
				if (entity_ptr != nullptr) {
					// Do not take item away from player unless item was successfully placed
					if (!placement_c::place_entity_at_coords_ranged(entity_ptr, tile_selected.first, tile_selected.second))
						return;
					
					player_manager::decrement_selected_item();
				}
			}
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS);

		// Remove entities resource
		input_manager::subscribe([]() {
			const auto tile_selected = mouse_selection::get_mouse_tile_coords();
			const auto* tile = world_manager::get_tile_world_coords(tile_selected.first, tile_selected.second);
			
			const auto* entity_ptr = tile->entity;
			// Pickup entity has priority over extract resource
			if (entity_ptr == nullptr) {
				// Extract resource
				auto* resource_tile = static_cast<data::Resource_tile*>(
					tile->get_tile_layer_tile_prototype(Chunk_tile::chunk_layer::resource));
				if (resource_tile != nullptr) {
					LOG_MESSAGE(debug, "MINING AWAY p");
				}
				
				return;
			}

			// Pickup entity
			test_rm_counter++;
			if (test_rm_counter >= 10) {
				if (placement_c::place_entity_at_coords_ranged(nullptr, tile_selected.first, tile_selected.second)) {
					test_rm_counter = 0;
					auto item_stack = data::item_stack(entity_ptr->get_item(), 1);
					inventory_c::add_itemstack_to_inv(
						player_manager::player_inventory, player_manager::inventory_size, item_stack);
					// TODO do something if the inventory is full
				}
			}
		}, GLFW_MOUSE_BUTTON_2, GLFW_PRESS);
	}

	
	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	unsigned short logic_tick = 1;
	while (!logic_loop_should_terminate) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		{
			// Do things every logic loop tick
			Event::raise<Logic_tick_event>(event_type::logic_tick, logic_tick);

			input_manager::raise();

			// Generate chunks
			world_generator::gen_chunk();

			mouse_selection::draw_cursor_overlay();
		}

		if (++logic_tick > 60)
			logic_tick = 1;

		next_frame += std::chrono::nanoseconds(16666666);
		std::this_thread::sleep_until(next_frame);
	}
}

void jactorio::game::terminate_logic_loop() {
	logic_loop_should_terminate = true;
}
