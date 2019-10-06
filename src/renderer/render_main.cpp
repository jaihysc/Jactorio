#include <GL/glew.h>

#include "renderer/render_main.h"

#include <vector>

#include "renderer/manager/imgui_manager.h"
#include "renderer/manager/window_manager.h"
#include "renderer/manager/shader_manager.h"

#include "renderer/opengl/shader.h"
#include "renderer/opengl/texture.h"
#include "renderer/rendering/renderer.h"

// Main draw loop is here
void renderer_main() {
	if (jactorio::renderer::opengl_init() != 0)
		return;

	GLFWwindow* window = jactorio::renderer::opengl_get_window();
	jactorio::renderer::setup(window);

	// #################################################################
	// Enables transparency in textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const jactorio::renderer::Shader shader(
		std::vector<jactorio::renderer::Shader_creation_input> {
			{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
			{"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER}
		}
	);
	shader.bind();
	jactorio::renderer::set_mvp_uniform_location(shader.get_uniform_location("u_model_view_projection_matrix"));

	// #################################################################

	jactorio::renderer::Renderer renderer{};

	// #################################################################

	// Loading textures
	const jactorio::renderer::Texture texture("data/core/graphics/background-image.png");
	texture.bind();

	// Texture is bound to slot 0 above, tell this to shader
	jactorio::renderer::Shader::set_uniform_1i(shader.get_uniform_location("u_texture"), 0);

	// #################################################################

	// Update game 60 times per second
	constexpr float update_interval = 1.f / 60;
	double last_time = 0.f;
	while (!glfwWindowShouldClose(window)) {
		// Update interval = 60 times / second
		if (glfwGetTime() - last_time > update_interval) {
			last_time = glfwGetTime();

			renderer.clear();
		
			// Draw
			renderer.draw(glm::vec3(0, 0, 0));
			// renderer.draw(va, ib, glm::vec3(10, 10, 0));

			jactorio::renderer::imgui_draw();
			
			jactorio::renderer::update_shader_mvp();
			glfwSwapBuffers(window);  // Done rendering

			glfwPollEvents();
		}
	}

	jactorio::renderer::imgui_terminate();
	jactorio::renderer::opengl_terminate();
}