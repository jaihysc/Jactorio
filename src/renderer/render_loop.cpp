#include "renderer/render_loop.h"

#include "renderer/rendering/renderer.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/window/window_manager.h"
#include "renderer/rendering/world_renderer.h"
#include "renderer/rendering/mvp_manager.h"

void jactorio::renderer::render_loop(Renderer* renderer) {
	Renderer::clear();

	world_renderer::render_player_position(renderer);
	// Set view matrix
	mvp_manager::update_view_transform();
	// Set projection matrix
	renderer->update_tile_projection_matrix();
	// Update matrices
	update_shader_mvp();
	renderer->draw(glm::vec3(0, 0, 0));

	imgui_draw();
	
	glfwSwapBuffers(window_manager::get_window());  // Done rendering
	glfwPollEvents();
}
