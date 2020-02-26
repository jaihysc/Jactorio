#include "game/logic_loop.h"

#include <chrono>
#include <thread>
#include <data/prototype/entity/transport/transport_line.h>

#include "jactorio.h"

#include "renderer/gui/imgui_manager.h"

#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "game/world/world_generator.h"
#include "game/world/world_manager.h"
#include "game/logic/transport_line_controller.h"

#include "data/data_manager.h"
#include "data/prototype/item/item.h"

bool logic_loop_should_terminate = false;
constexpr float move_speed = 0.8f;

void jactorio::game::init_logic_loop(std::mutex* mutex) {
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
			// If a layer is already activated, deactivate it, otherwise open the gui menu
			if (player_manager::get_activated_layer() != nullptr)
				player_manager::set_activated_layer(nullptr);
			else
				set_window_visibility(gui_window::character, !get_window_visibility(gui_window::character));
		}, GLFW_KEY_TAB, GLFW_RELEASE);
	}

	{
		// Place entities
		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured)
				return;

			const auto tile_selected = mouse_selection::get_mouse_tile_coords();
			player_manager::try_place(tile_selected.first, tile_selected.second);
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS);

		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured)
				return;

			const auto tile_selected = mouse_selection::get_mouse_tile_coords();
			player_manager::try_place(tile_selected.first, tile_selected.second, true);
		}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS_FIRST);

		// Remove entities or mine resource
		input_manager::subscribe([]() {
			if (renderer::imgui_manager::input_captured)
				return;

			const auto tile_selected = mouse_selection::get_mouse_tile_coords();
			player_manager::try_pickup(tile_selected.first, tile_selected.second);
		}, GLFW_MOUSE_BUTTON_2, GLFW_PRESS);
	}


	// TODO remove this
	input_manager::subscribe([]() {
		Event::subscribe_once(event_type::logic_tick, []() {
			auto* chunk = world_manager::get_chunk(-1, 0);
			auto* chunk_2 = world_manager::get_chunk(0, 0);

			auto* chunk_3 = world_manager::get_chunk(-1, 1);
			auto* chunk_4 = world_manager::get_chunk(0, 1);

			auto& logic_chunk = world_manager::logic_add_chunk(chunk);
			world_manager::logic_add_chunk(chunk_2);
			world_manager::logic_add_chunk(chunk_3);
			world_manager::logic_add_chunk(chunk_4);

			auto* proto =
				data::data_manager::data_raw_get<data::Item>(data::data_category::item,
															 "__base__/transport-belt-basic-item");

			auto* belt_proto =
				data::data_manager::data_raw_get<data::Transport_line>(data::data_category::transport_belt,
																	   "__base__/transport-belt-basic");

			// Segments (Logic chunk must be created first)
			auto* up_segment = new jactorio::game::Transport_line_segment(
				jactorio::game::Transport_line_segment::move_dir::up,
				jactorio::game::Transport_line_segment::terminationType::bend_right,
				6);
			auto* right_segment = new jactorio::game::Transport_line_segment(
				jactorio::game::Transport_line_segment::move_dir::right,
				jactorio::game::Transport_line_segment::terminationType::bend_right,
				5);
			auto* down_segment = new jactorio::game::Transport_line_segment(
				jactorio::game::Transport_line_segment::move_dir::down,
				jactorio::game::Transport_line_segment::terminationType::bend_right,
				6);
			auto* left_segment = new jactorio::game::Transport_line_segment(
				jactorio::game::Transport_line_segment::move_dir::left,
				jactorio::game::Transport_line_segment::terminationType::bend_right,
				5);

			// What each transport segment empties into
			up_segment->target_segment = right_segment;
			right_segment->target_segment = down_segment;
			down_segment->target_segment = left_segment;
			left_segment->target_segment = up_segment;

			auto& up = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
				.emplace_back(belt_proto, 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
				.emplace_back(belt_proto, 4, 0);
			right.unique_data = right_segment;

			auto& down = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
				.emplace_back(belt_proto, 4, 5);
			down.unique_data = down_segment;

			auto& left = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
				.emplace_back(belt_proto, 0, 5);
			left.unique_data = left_segment;

			// Insert item
			for (int i = 0; i < 100; ++i) {
//				transport_line_c::belt_insert_item(true, left_segment, 0.f, proto);
				transport_line_c::belt_insert_item(false, left_segment, 0.f, proto);
			}
		});
	}, GLFW_KEY_1, GLFW_RELEASE);
	input_manager::subscribe([]() {
		Event::subscribe_once(event_type::logic_tick, []() {
			auto* chunk = world_manager::get_chunk(0, 0);

		});
	}, GLFW_KEY_2, GLFW_PRESS);


	// Runtime
	auto next_frame = std::chrono::steady_clock::now();
	unsigned short logic_tick = 1;
	while (!logic_loop_should_terminate) {
		EXECUTION_PROFILE_SCOPE(logic_loop_timer, "Logic loop");

		{
//			std::lock_guard lk(*mutex);

			mouse_selection::calculate_mouse_tile_coords();

			// Do things every logic loop tick
			Event::raise<Logic_tick_event>(event_type::logic_tick, logic_tick);

			input_manager::raise();

			// Generate chunks
			world_generator::gen_chunk();

			mouse_selection::draw_cursor_overlay();

			player_manager::recipe_craft_tick();

			// Logistics logic
			{
				EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

				std::queue<transport_line_c::Segment_transition_item> queue;
				for (auto& logic_chunk : world_manager::logic_get_all_chunks()) {
					transport_line_c::logic_update(queue, &logic_chunk.second);
				}
				transport_line_c::logic_process_queued_items(queue);
			}
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
