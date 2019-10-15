#include "renderer/render_loop.h"

#include "renderer/rendering/renderer.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/window/window_manager.h"

void jactorio::renderer::render_loop(Renderer* renderer) {
	renderer->clear();

	// Draw
	renderer->draw(glm::vec3(0, 0, 0));
	// renderer.draw(va, ib, glm::vec3(10, 10, 0));

	imgui_draw();
	
	update_shader_mvp();
	glfwSwapBuffers(window_manager::get_window());  // Done rendering

	glfwPollEvents();
}
