#include <GL/glew.h>

#include "renderer/render_main.h"

#include <vector>
#include <thread>

#include "core/debug/execution_timer.h"
#include "core/resource_guard.h"
#include "core/logger.h"
#include "data/data_manager.h"

#include "renderer/gui/imgui_manager.h"
#include "renderer/window/window_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/texture.h"
#include "renderer/rendering/spritemap_generator.h"
#include "renderer/render_loop.h"

#include "game/input/input_manager.h"
#include "game/world/world_manager.h"
#include "game/logic_loop.h"
#include "core/filesystem.h"
#include "data/pybind/pybind_manager.h"

bool refresh_renderer = false;
bool clear_chunk_data = false;
unsigned short window_x = 0;
unsigned short window_y = 0;

jactorio::renderer::Renderer* main_renderer = nullptr;
bool render_draw = false;

// Called every renderer cycle, cannot put code in callback due to single thread of opengl
bool toggle_fullscreen = false;

void jactorio::renderer::set_recalculate_renderer(const unsigned short window_size_x,
                                                  const unsigned short window_size_y) {
	window_x = window_size_x;
	window_y = window_size_y;

	refresh_renderer = true;
}

jactorio::renderer::Renderer* jactorio::renderer::get_base_renderer() {
	return main_renderer;
}

int jactorio::renderer::render_init() {
	auto loop_termination_guard = core::Resource_guard(&game::terminate_logic_loop);
	auto window_manager_guard = core::Resource_guard(window_manager::terminate);

	if (window_manager::init(640, 490) != 0)
		return 1;

	
	GLFWwindow* window = window_manager::get_window();

	auto imgui_manager_guard = core::Resource_guard(&imgui_manager::imgui_terminate);
	imgui_manager::setup(window);

	// Load prototype data
	if (data::data_manager::load_data(core::filesystem::resolve_path("~/data")) != 0) {
		// error occurred
		imgui_manager::show_error_prompt(
			"Failed to load prototype(s)", data::pybind_manager::get_last_error_message());
		return 2;
	}
	

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
	auto sprite_map_guard = core::Resource_guard(&renderer_sprites::clear_spritemaps);
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
		toggle_fullscreen = true;
	}, GLFW_KEY_SPACE, GLFW_RELEASE);

	game::input_manager::subscribe([]() {
		Renderer::tile_width++;
		refresh_renderer = true;
	}, GLFW_KEY_Z, GLFW_RELEASE);
	game::input_manager::subscribe([]() {
		if (Renderer::tile_width > 1) {
			Renderer::tile_width--;
			refresh_renderer = true;
		}

	}, GLFW_KEY_X, GLFW_RELEASE);

	game::input_manager::subscribe([]() {
		clear_chunk_data = true;
	}, GLFW_KEY_R, GLFW_RELEASE);


	// Main rendering loop
	{
		LOG_MESSAGE(info, "2 - Runtime stage")

		auto renderer_guard = core::Resource_guard<void>([]() { delete main_renderer; });
		main_renderer = new Renderer();
		
		auto chunk_data_guard = core::Resource_guard(&game::world_manager::clear_chunk_data);

		// core::loop_manager::render_loop_ready(renderer_draw);

		auto next_tick = std::chrono::steady_clock::now();
		while (!glfwWindowShouldClose(window)) {
			EXECUTION_PROFILE_SCOPE(render_loop_timer, "Render loop");

			// Think of a better way to toggle fullscreen?
			if (toggle_fullscreen) {
				toggle_fullscreen = false;
				window_manager::set_fullscreen(!window_manager::is_fullscreen());
			}
			if (refresh_renderer) {
				main_renderer->recalculate_buffers(window_x, window_y);
				refresh_renderer = false;
			}
			if (clear_chunk_data) {
				clear_chunk_data = false;
				game::world_manager::clear_chunk_data();
			}

			render_draw = false;
			// Don't multi-thread opengl
			render_loop(main_renderer);


			next_tick += std::chrono::microseconds(16666);
			std::this_thread::sleep_until(next_tick);
		}
	}

	return 0;
}
