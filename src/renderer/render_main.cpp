#include <GL/glew.h>

#include "renderer/render_main.h"

#include <vector>

#include "data/data_manager.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/window/window_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/texture.h"
#include "renderer/rendering/renderer.h"
#include "renderer/rendering/renderer_sprites.h"
#include "renderer/render_loop.h"
#include "core/logger.h"
#include "game/world/world_manager.h"

bool refresh_renderer = false;
unsigned short window_x = 0;
unsigned short window_y = 0;

unsigned short render_refresh_rate = 60;
float render_update_interval = 1.f / render_refresh_rate;

void jactorio::renderer::set_recalculate_renderer(const unsigned short window_size_x,
                                                  const unsigned short window_size_y) {
	window_x = window_size_x;
	window_y = window_size_y;

	refresh_renderer = true;
}


void jactorio::renderer::set_render_refresh_rate(const unsigned short refresh_rate) {
	render_refresh_rate = refresh_rate;
	render_update_interval = 1.f / render_refresh_rate;
}

unsigned short jactorio::renderer::get_render_refresh_rate() {
	return render_refresh_rate;
}


void jactorio::renderer::renderer_main() {
	if (window_manager::init(640, 490) != 0)
		return;

	GLFWwindow* window = window_manager::get_window();
	setup(window);

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
	std::vector<data::Prototype_base> texture_paths = data::data_manager::get_all_data(
		data::data_category::tile);

	const auto r_sprites = Renderer_sprites{};
	const Renderer_sprites::Spritemap_data spritemap_data = r_sprites.gen_spritemap(
		texture_paths.data(), texture_paths.size());

	const Texture texture(spritemap_data.spritemap);
	texture.bind(0);

	
	auto renderer = new Renderer(spritemap_data.sprite_positions);
	double render_last_time = 0.f;


	// World generator test
	auto* tiles = new game::Tile[1024];
	for (int i = 0; i < 32 * 32; ++i) {
		const auto proto_tile = data::data_manager::data_raw_get(data::data_category::tile, "test_tile");

		tiles[i].tile_prototype = static_cast<data::Tile*>(proto_tile);
	}
	
	game::world_manager::add_chunk(new game::Chunk{0, 0, tiles});
	

	log_message(core::logger::info, "Jactorio", "2 - Runtime stage");
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetTime() - render_last_time > render_update_interval) {
			render_last_time = glfwGetTime();

			game::world_manager::draw_chunks(*renderer, 
			                                 10, 20, 
			                                 0, 0, 
			                                 1, 1);
			
			// Don't multi-thread opengl
			render_loop(renderer);

			// Swap renderers if a new one is placed in swap
			if (refresh_renderer) {
				renderer->recalculate_buffers(window_x, window_y);
				refresh_renderer = false;
			}
		}
	}

	imgui_terminate();
	window_manager::terminate();
}
