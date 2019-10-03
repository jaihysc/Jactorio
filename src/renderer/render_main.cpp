#include <GL/glew.h>

#include "renderer/render_main.h"

#include <vector>

#include "renderer/manager/imgui_manager.h"
#include "renderer/manager/mvp_manager.h"
#include "renderer/manager/window_manager.h"
#include "renderer/manager/shader_manager.h"

#include "renderer/opengl/vertex_array.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/texture.h"
#include "renderer/rendering/renderer.h"

// Main draw loop is here
void renderer_main() {
	if (jactorio_renderer::opengl_init() != 0)
		return;

	GLFWwindow* window = jactorio_renderer::opengl_get_window();
	jactorio_renderer::setup(window);

	// Update game logic 60 times per second
	constexpr float logic_update_interval = 1.f / 60;

	// #################################################################
	// Enables transparency in textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const jactorio_renderer_gl::Shader shader(
		std::vector<jactorio_renderer_gl::Shader_creation_input> {
			{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
			{"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER}
		}
	);
	shader.bind();
	jactorio_renderer::set_mvp_uniform_location(shader.get_uniform_location("u_model_view_projection_matrix"));

	jactorio_renderer::set_proj_calculation_tile_width(32);
	jactorio_renderer::calculate_tile_properties();
	jactorio_renderer::setg_projection_matrix(jactorio_renderer::get_proj_matrix());

	// #################################################################

	jactorio_renderer_rendering::Renderer renderer{};

	// #################################################################

	// Loading textures
	const jactorio_renderer_gl::Texture texture("data/core/graphics/background-image-logo.png");
	texture.bind();

	// Texture is bound to slot 0 above, tell this to shader
	jactorio_renderer_gl::Shader::set_uniform_1i(shader.get_uniform_location("u_texture"), 0);

	// #################################################################
	
	double last_time = 0.f;
	while (!glfwWindowShouldClose(window)) {
		// Update interval = 60 times / second
		renderer.clear();
		
		// Update logic every logic_update_interval
		if (glfwGetTime() - last_time > logic_update_interval) {
			last_time = glfwGetTime();
		}

		// Draw
		renderer.draw(glm::vec3(0, 0, 0));
		// renderer.draw(va, ib, glm::vec3(10, 10, 0));

		jactorio_renderer::draw();
		
		jactorio_renderer::update_shader_mvp();
		glfwSwapBuffers(window);  // Done rendering
		glfwPollEvents();
	}

	jactorio_renderer::terminate();
	jactorio_renderer::opengl_terminate();
}