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


void jactorio::renderer::render_init() {
	if (window_manager::init(640, 490) != 0)
		return;

	auto window_manager_guard = core::Resource_guard(window_manager::terminate);
	GLFWwindow* window = window_manager::get_window();
	
	// #################################################################
	// Enables transparency in textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	std::vector<data::Sprite*> texture_paths = 
		data::data_manager::data_raw_get_all<data::Sprite>(data::data_category::sprite);

	const auto r_sprites = Renderer_sprites{};
	const Renderer_sprites::Spritemap_data spritemap_data = r_sprites.gen_spritemap(
		texture_paths.data(), texture_paths.size());

	const Texture texture(spritemap_data.spritemap);
	texture.bind(0);

	main_renderer = new Renderer();
	Renderer::set_spritemap_coords(spritemap_data.sprite_positions);
	
	game::input_manager::register_input_callback([]() {
		glfwSetWindowShouldClose(window_manager::get_window(), GL_TRUE);
		
	}, GLFW_KEY_ESCAPE, GLFW_RELEASE);

	game::input_manager::register_input_callback([]() {
		toggle_fullscreen = true;
	}, GLFW_KEY_SPACE, GLFW_RELEASE);

	game::input_manager::register_input_callback([]() {
		main_renderer->tile_width++;
		refresh_renderer = true;
	}, GLFW_KEY_Z, GLFW_RELEASE);
	game::input_manager::register_input_callback([]() {
		if (main_renderer->tile_width > 1) {
			main_renderer->tile_width--;
			refresh_renderer = true;
		}

	}, GLFW_KEY_X, GLFW_RELEASE);

	game::input_manager::register_input_callback([]() {
		clear_chunk_data = true;
	}, GLFW_KEY_R, GLFW_RELEASE);

	// #################################################################
	{
		LOG_MESSAGE(info, "2 - Runtime stage")

		auto imgui_manager_guard = core::Resource_guard(&imgui_manager::imgui_terminate);
		imgui_manager::setup(window);

		auto chunk_data_guard = core::Resource_guard(&game::world_manager::clear_chunk_data);

		auto loop_termination_guard = core::Resource_guard(&game::terminate_logic_loop);
		
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

}
