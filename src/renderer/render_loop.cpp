#include "renderer/render_loop.h"

#include "core/debug/execution_timer.h"
#include "renderer/rendering/renderer.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/window/window_manager.h"
#include "renderer/rendering/world_renderer.h"

void jactorio::renderer::render_loop(Renderer* renderer) {
	EXECUTION_PROFILE_SCOPE(render_preparation_timer, "Render preparation");
	
	Renderer::clear();

	// MVP Matricies updated in here
	world_renderer::render_player_position(renderer);

	imgui_manager::imgui_draw();
	
	glfwSwapBuffers(window_manager::get_window());  // Done rendering
	glfwPollEvents();
}
