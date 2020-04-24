// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <GL/glew.h>

#include "renderer/render_main.h"

#include <thread>
#include <vector>
#include <GLFW/glfw3.h>

#include "jactorio.h"
#include "core/resource_guard.h"

#include "renderer/gui/imgui_manager.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/texture.h"
#include "renderer/rendering/spritemap_generator.h"
#include "renderer/rendering/world_renderer.h"
#include "renderer/window/window_manager.h"

#include "game/game_data.h"
#include "game/logic_loop.h"
#include "game/event/event.h"
#include "game/input/input_manager.h"

unsigned short window_x = 0;
unsigned short window_y = 0;

jactorio::renderer::Renderer* main_renderer = nullptr;

void jactorio::renderer::set_recalculate_renderer(const unsigned short window_size_x,
                                                  const unsigned short window_size_y) {
	window_x = window_size_x;
	window_y = window_size_y;

	game::game_data->event.subscribe_once(game::eventType::renderer_tick, []() {
		main_renderer->recalculate_buffers(window_x, window_y);
	});
}

jactorio::renderer::Renderer* jactorio::renderer::get_base_renderer() {
	return main_renderer;
}

void jactorio::renderer::render_init() {
	core::Resource_guard<void> loop_termination_guard([]() {
		render_thread_should_exit = true;
		game::logic_thread_should_exit = true;
	});

	// Init window
	core::Resource_guard window_manager_guard(&window_manager::terminate);
	try {
		if (window_manager::init(840, 490) != 0)
			return;
	}
	catch (data::Data_exception&) {
		return;
	}


	core::Resource_guard imgui_manager_guard(&imgui_manager::imgui_terminate);
	imgui_manager::setup(window_manager::get_window());

	// Shader
	const Shader shader(
		std::vector<Shader_creation_input>{
			{"~/data/core/shaders/vs.vert", GL_VERTEX_SHADER},
			{"~/data/core/shaders/fs.frag", GL_FRAGMENT_SHADER}
		}
	);
	shader.bind();
	set_mvp_uniform_location(
		shader.get_uniform_location("u_model_view_projection_matrix"));

	// Texture will be bound to slot 0 above, tell this to shader
	Shader::set_uniform_1i(shader.get_uniform_location("u_texture"), 0);


	// Loading textures
	auto renderer_sprites = Renderer_sprites();
	renderer_sprites.create_spritemap(data::Sprite::spriteGroup::terrain, true);
	renderer_sprites.create_spritemap(data::Sprite::spriteGroup::gui, false);

	// Terrain
	Renderer::set_spritemap_coords(renderer_sprites.get_spritemap(data::Sprite::spriteGroup::terrain).sprite_positions);
	renderer_sprites.get_texture(data::Sprite::spriteGroup::terrain)->bind(0);

	// Gui
	imgui_manager::setup_character_data(renderer_sprites);


	// ======================================================================
	
	game::game_data->input.key.subscribe([]() {
		game::game_data->event.subscribe_once(game::eventType::renderer_tick, []() {
			window_manager::set_fullscreen(!window_manager::is_fullscreen());
			main_renderer->recalculate_buffers(window_x, window_y);
		});
	}, game::inputKey::space, game::inputAction::key_down);

	// Main rendering loop
	{
		LOG_MESSAGE(info, "2 - Runtime stage")

		// From my testing, allocating it on the heap is faster than using the stack
		core::Resource_guard<void> renderer_guard([]() { delete main_renderer; });
		main_renderer = new Renderer();


		auto next_frame = std::chrono::steady_clock::now();  // For zeroing the time
		while (!render_thread_should_exit) {
			EXECUTION_PROFILE_SCOPE(render_loop_timer, "Render loop");

			// ======================================================================
			// RENDER LOOP ======================================================================
			{
				EXECUTION_PROFILE_SCOPE(logic_update_timer, "Render update");

				glfwPollEvents();
				if (glfwWindowShouldClose(window_manager::get_window()))
					render_thread_should_exit = true;

				game::game_data->event.raise<game::Renderer_tick_event>(game::eventType::renderer_tick);

				// ======================================================================
				// World
				Renderer::g_clear();

				// MVP Matrices updated in here
				// Mutex locks in function call
				world_renderer::render_player_position(
					game::game_data->world,
					main_renderer,
					game::game_data->player.get_player_position_x(), game::game_data->player.get_player_position_y());

				// ======================================================================
				// Gui
				{
					std::lock_guard<std::mutex> guard{game::game_data->player.mutex};

					imgui_manager::imgui_draw(game::game_data->player, game::game_data->event);
				}
			}
			// ======================================================================
			// ======================================================================

			// Sleep until the next fixed update interval
			auto time_end = std::chrono::steady_clock::now();
			while (time_end > next_frame) {
				next_frame += std::chrono::nanoseconds(16666666);
			}
			std::this_thread::sleep_until(next_frame);

			glfwSwapBuffers(window_manager::get_window());
		}
	}

	LOG_MESSAGE(info, "Renderer thread exited");
}
