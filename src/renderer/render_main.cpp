// 
// render_main.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/15/2019
// Last modified: 03/20/2020
// 

#include <GL/glew.h>

#include "renderer/render_main.h"

#include <thread>
#include <vector>

#include "core/debug/execution_timer.h"
#include "core/logger.h"
#include "core/resource_guard.h"

#include "renderer/gui/imgui_manager.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/texture.h"
#include "renderer/rendering/spritemap_generator.h"
#include "renderer/rendering/world_renderer.h"
#include "renderer/window/window_manager.h"

#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/logic_loop.h"
#include "game/game_data.h"

unsigned short window_x = 0;
unsigned short window_y = 0;

jactorio::renderer::Renderer* main_renderer = nullptr;

void jactorio::renderer::set_recalculate_renderer(const unsigned short window_size_x,
                                                  const unsigned short window_size_y) {
	window_x = window_size_x;
	window_y = window_size_y;

	game::Event::subscribe_once(game::event_type::renderer_tick, []() {
		main_renderer->recalculate_buffers(window_x, window_y);
	});
}

jactorio::renderer::Renderer* jactorio::renderer::get_base_renderer() {
	return main_renderer;
}

int jactorio::renderer::render_init(std::mutex* mutex) {
	core::Resource_guard<void> loop_termination_guard([]() {
		render_thread_should_exit = true;
		game::logic_thread_should_exit = true;
	});
	core::Resource_guard window_manager_guard(&window_manager::terminate);

	if (window_manager::init(640, 490) != 0)
		return 1;


	GLFWwindow* window = window_manager::get_window();

	core::Resource_guard imgui_manager_guard(&imgui_manager::imgui_terminate);
	imgui_manager::setup(window);


	// Enables transparency in textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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
	core::Resource_guard sprite_map_guard(&renderer_sprites::clear_spritemaps);
	renderer_sprites::create_spritemap(data::Sprite::sprite_group::terrain, true);
	renderer_sprites::create_spritemap(data::Sprite::sprite_group::gui, false);

	// Terrain
	Renderer::set_spritemap_coords(
		renderer_sprites::get_spritemap(data::Sprite::sprite_group::terrain).sprite_positions);
	renderer_sprites::get_texture(data::Sprite::sprite_group::terrain)->bind(0);

	// Gui
	imgui_manager::setup_character_data();

	// TODO Temporary keybinds, move this elsewhere
	game::input_manager::subscribe([]() {
		glfwSetWindowShouldClose(window_manager::get_window(), GL_TRUE);

	}, GLFW_KEY_ESCAPE, GLFW_RELEASE);

	game::input_manager::subscribe([]() {
		game::Event::subscribe_once(game::event_type::renderer_tick, []() {
			window_manager::set_fullscreen(!window_manager::is_fullscreen());
			main_renderer->recalculate_buffers(window_x, window_y);
		});
	}, GLFW_KEY_SPACE, GLFW_RELEASE);

	// game::input_manager::subscribe([]() {
	// 	Renderer::tile_width++;
	// 	game::Event::subscribe_once(game::event_type::renderer_tick, []() {
	// 		main_renderer->recalculate_buffers(window_x, window_y);
	// 	});
	// 	
	// }, GLFW_KEY_Z, GLFW_RELEASE);
	// game::input_manager::subscribe([]() {
	// 	if (Renderer::tile_width > 1) {
	// 		Renderer::tile_width--;
	// 		game::Event::subscribe_once(game::event_type::renderer_tick, []() {
	// 			main_renderer->recalculate_buffers(window_x, window_y);
	// 		});
	// 	}
	//
	// }, GLFW_KEY_X, GLFW_RELEASE);
	//
	game::input_manager::subscribe([]() {
		game::Event::subscribe_once(game::event_type::renderer_tick, []() {
			game::game_data->player.get_player_world().clear_chunk_data();
		});
	}, GLFW_KEY_P, GLFW_RELEASE);

	// Main rendering loop
	{
		LOG_MESSAGE(info, "2 - Runtime stage")

		// From my testing, allocating it on the heap is faster than using the stack
		core::Resource_guard<void> renderer_guard([]() { delete main_renderer; });
		main_renderer = new Renderer();

		auto next_tick = std::chrono::steady_clock::now();
		while (!render_thread_should_exit) {
			EXECUTION_PROFILE_SCOPE(render_loop_timer, "Render loop");

			{
				//				std::lock_guard lk(*mutex);

				game::Event::raise<game::Renderer_tick_event>(game::event_type::renderer_tick);

				Renderer::g_clear();

				// MVP Matrices updated in here
				world_renderer::render_player_position(
					game::game_data->world,
					main_renderer,
					game::game_data->player.get_player_position_x(), game::game_data->player.get_player_position_y());

				imgui_manager::imgui_draw(game::game_data->player);

				glfwSwapBuffers(window_manager::get_window());  // Done rendering
				glfwPollEvents();
			}

			if (glfwWindowShouldClose(window))
				render_thread_should_exit = true;

			next_tick += std::chrono::microseconds(16666);
			std::this_thread::sleep_until(next_tick);
		}
	}

	LOG_MESSAGE(debug, "Renderer thread exited");
	return 0;
}
