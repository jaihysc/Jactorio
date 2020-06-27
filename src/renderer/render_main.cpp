// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <GL/glew.h>

#include "renderer/render_main.h"

#include <SDL.h>
#include <thread>
#include <vector>

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

using namespace jactorio;

unsigned short window_x = 0;
unsigned short window_y = 0;

renderer::Renderer* main_renderer = nullptr;

void renderer::SetRecalculateRenderer(const unsigned short window_size_x,
                                      const unsigned short window_size_y) {
	window_x = window_size_x;
	window_y = window_size_y;

	game::game_data->event.SubscribeOnce(game::EventType::renderer_tick, []() {
		main_renderer->GRecalculateBuffers(window_x, window_y);
	});
}

renderer::Renderer* renderer::GetBaseRenderer() {
	return main_renderer;
}


void RenderingLoop() {
	LOG_MESSAGE(info, "2 - Runtime stage")

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
			renderer::Renderer::GClear();

			// MVP Matrices updated in here
			// Mutex locks in function call
			RenderPlayerPosition(
				game::game_data->world,
				main_renderer,
				game::game_data->player.GetPlayerPositionX(), game::game_data->player.GetPlayerPositionY());

			// ======================================================================
			// Gui
			{
				std::lock_guard<std::mutex> guard{game::game_data->player.mutex};

				renderer::ImguiDraw(game::game_data->player, game::game_data->event);
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

		SDL_GL_SwapWindow(renderer::GetWindow());

		while (SDL_PollEvent(&e)) {
			renderer::HandleSdlEvent(e);
		}
	}
}

void renderer::RenderInit() {
	core::ResourceGuard<void> loop_termination_guard([]() {
		render_thread_should_exit      = true;
		game::logic_thread_should_exit = true;
	});

	// Init window
	core::ResourceGuard window_manager_guard(&TerminateWindow);
	try {
		if (InitWindow(840, 490) != 0)
			return;
	}
	catch (data::DataException&) {
		return;
	}


	core::ResourceGuard imgui_manager_guard(&ImguiTerminate);
	Setup(GetWindow());

	// Shader
	const Shader shader(
		std::vector<ShaderCreationInput>{
			{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
			{"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER}
		}
	);
	shader.Bind();
	SetMvpUniformLocation(
		shader.GetUniformLocation("u_model_view_projection_matrix"));

	// Texture will be bound to slot 0 above, tell this to shader
	Shader::SetUniform1I(shader.GetUniformLocation("u_texture"), 0);


	// ======================================================================
	// Accessing game data 

	// Since game data will be now accessed, wait until prototype loading is complete
	LOG_MESSAGE(debug, "Waiting for prototype loading to complete");
	while (!game::prototype_loading_complete)
		;
	LOG_MESSAGE(debug, "Continuing renderer initialization");


	// Loading textures
	auto renderer_sprites = RendererSprites();
	renderer_sprites.GInitializeSpritemap(data::Sprite::SpriteGroup::terrain, true);
	renderer_sprites.GInitializeSpritemap(data::Sprite::SpriteGroup::gui, false);

	// Terrain
	Renderer::SetSpritemapCoords(renderer_sprites.GetSpritemap(data::Sprite::SpriteGroup::terrain).spritePositions);
	renderer_sprites.GetTexture(data::Sprite::SpriteGroup::terrain)->Bind(0);

	// Gui
	SetupCharacterData(renderer_sprites);


	// ======================================================================

	game::game_data->input.key.Register([]() {
		game::game_data->event.SubscribeOnce(game::EventType::renderer_tick, []() {
			SetFullscreen(!IsFullscreen());
			main_renderer->GRecalculateBuffers(window_x, window_y);
		});
	}, SDLK_KP_SPACE, game::InputAction::key_down);

	RenderingLoop();

	LOG_MESSAGE(info, "Renderer thread exited");
}
