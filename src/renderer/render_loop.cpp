// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/render_loop.h"

#include <chrono>
#include <thread>

#include <SDL.h>
#include <examples/imgui_impl_sdl.h>

#include "jactorio.h"
#include "core/resource_guard.h"

#include "renderer/display_window.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/opengl/texture.h"
#include "renderer/rendering/spritemap_generator.h"

#include "game/game_data.h"
#include "game/logic_loop.h"
#include "game/event/event.h"
#include "game/input/input_manager.h"

using namespace jactorio;

unsigned int window_x = 0;
unsigned int window_y = 0;

renderer::Renderer* main_renderer = nullptr;

void renderer::ChangeWindowSize(const unsigned int window_size_x,
                                const unsigned int window_size_y) {
	window_x = window_size_x;
	window_y = window_size_y;

	game::game_data->event.SubscribeOnce(game::EventType::renderer_tick, []() {
		main_renderer->GlResizeBuffers(window_x, window_y);
	});
}

renderer::Renderer* renderer::GetBaseRenderer() {
	return main_renderer;
}


void RenderingLoop(const renderer::DisplayWindow& display_window) {
	LOG_MESSAGE(info, "2 - Runtime stage");

	// From my testing, allocating it on the heap is faster than using the stack
	core::ResourceGuard<void> renderer_guard([]() { delete main_renderer; });
	main_renderer = new renderer::Renderer();


	auto next_frame = std::chrono::steady_clock::now();  // For zeroing the time

	SDL_Event e;

	while (!renderer::render_thread_should_exit) {
		EXECUTION_PROFILE_SCOPE(render_loop_timer, "Render loop");

		// ======================================================================
		// RENDER LOOP ======================================================================
		{
			EXECUTION_PROFILE_SCOPE(logic_update_timer, "Render update");

			game::game_data->event.Raise<game::RendererTickEvent>(game::EventType::renderer_tick);

			// ======================================================================
			// World
			{
				renderer::Renderer::GlClear();
				std::lock_guard<std::mutex> guard{game::game_data->world.worldDataMutex};

				// MVP Matrices updated in here
				main_renderer->RenderPlayerPosition(
					game::game_data->logic.GameTick(),
					game::game_data->world,
					game::game_data->player.GetPlayerPositionX(), game::game_data->player.GetPlayerPositionY()
				);
			}

			// ======================================================================
			// Gui
			{
				std::lock_guard<std::mutex> guard{game::game_data->player.mutex};

				ImguiDraw(display_window, game::game_data->player, game::game_data->prototype, game::game_data->event);
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

		SDL_GL_SwapWindow(display_window.GetWindow());

		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			display_window.HandleSdlEvent(e);
		}
	}
}

void renderer::RenderInit() {
	core::ResourceGuard<void> loop_termination_guard([]() {
		render_thread_should_exit      = true;
		game::logic_thread_should_exit = true;
	});

	// Init window
	DisplayWindow display_window{};
	try {
		if (display_window.Init(840, 490) != 0)
			return;
	}
	catch (data::DataException&) {
		return;
	}


	core::ResourceGuard imgui_manager_guard(&ImguiTerminate);
	Setup(display_window);

	// Shader
	const Shader shader(
		std::vector<ShaderCreationInput>{
			{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
			{"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER}
		}
	);
	shader.Bind();
	SetMvpUniformLocation(shader.GetUniformLocation("u_model_view_projection_matrix"));

	// Texture will be bound to slot 0 above, tell this to shader
	Shader::SetUniform1I(shader.GetUniformLocation("u_texture"), 0);


	// ======================================================================
	// Accessing game data 

	// Since game data will be now accessed, wait until prototype loading is complete
	LOG_MESSAGE(debug, "Waiting for prototype loading to complete");
	while (!game::prototype_loading_complete);
	LOG_MESSAGE(debug, "Continuing renderer initialization");


	// Loading textures
	auto renderer_sprites = RendererSprites();
	renderer_sprites.GInitializeSpritemap(game::game_data->prototype, data::Sprite::SpriteGroup::terrain, true);
	renderer_sprites.GInitializeSpritemap(game::game_data->prototype, data::Sprite::SpriteGroup::gui, false);

	// Terrain
	Renderer::SetSpritemapCoords(renderer_sprites.GetSpritemap(data::Sprite::SpriteGroup::terrain).spritePositions);
	renderer_sprites.GetTexture(data::Sprite::SpriteGroup::terrain)->Bind(0);

	// Gui
	SetupCharacterData(renderer_sprites);


	// ======================================================================

	game::game_data->input.key.Register([]() {
		game::game_data->event.SubscribeOnce(game::EventType::renderer_tick, []() {
			// display_window.SetFullscreen(!display_window.IsFullscreen());
			// main_renderer->GRecalculateBuffers(window_x, window_y);
		});
	}, SDLK_SPACE, game::InputAction::key_down);

	RenderingLoop(display_window);

	LOG_MESSAGE(info, "Renderer thread exited");
}
