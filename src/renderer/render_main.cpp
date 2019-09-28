#include <GL/glew.h>

#include "renderer/render_main.h"

#include <vector>

#include "renderer/manager/imgui_manager.h"
#include "renderer/manager/mvp_manager.h"
#include "renderer/manager/window_manager.h"
#include "renderer/manager/shader_manager.h"

#include "renderer/opengl/vertex_array.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/texture.h"

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
	
	const jactorio_renderer_gl::Vertex_array va{};

	// Use index buffers to avoid repeating oneself

	float triangle_coords[] = {
		0.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 1.f, 0.f,
		0.f, 1.f, 0.f,

		2.f, 2.f, 0.f,
		4.f, 2.f, 0.f,
		4.f, 4.f, 0.f,
		2.f, 4.f, 0.f,

		8.f, 8.f, 0.f,
		12.f, 8.f, 0.f,
		12.f, 12.f, 0.f,
		8.f, 12.f, 0.f
	};

	const jactorio_renderer_gl::Vertex_buffer vb(triangle_coords, 12 * 3 * sizeof(float));
	va.add_buffer(vb, 3, 0);

	float tex_coords[] = {
		0.f, 1.f,  // bottom left
		1.f, 1.f,  // bottom right
		1.f, 0.f,  // upper right
		0.f, 0.f,  // upper left

		0.f, 1.f,  // bottom left
		1.f, 1.f,  // bottom right
		1.f, 0.f,  // upper right
		0.f, 0.f,  // upper left

		0.f, 1.f,  // bottom left
		1.f, 1.f,  // bottom right
		1.f, 0.f,  // upper right
		0.f, 0.f,  // upper left
	};
	const jactorio_renderer_gl::Vertex_buffer vb2(tex_coords, 12 * 2 * sizeof(float));
	va.add_buffer(vb2, 2, 1);
	
	// #################################################################

	// Index buffer for optimization, avoids redundant vertices
	unsigned int triangle_cord_indices[] = {
		0, 1, 2,
		2, 3, 0,
		
		4, 5, 6,
		6, 7, 4,

		8, 9, 10,
		10, 11, 8
	};
	const jactorio_renderer_gl::Index_buffer ib(triangle_cord_indices, 18);

	// #################################################################

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

	// Loading textures
	const jactorio_renderer_gl::Texture texture("data/core/graphics/background-image-logo.png");
	texture.bind();

	// Texture is bound to slot 0 above, tell this to shader
	jactorio_renderer_gl::Shader::set_uniform_1i(shader.get_uniform_location("u_texture"), 0);

	// #################################################################
	
	// const Renderer renderer;
	
	double last_time = 0.f;
	while (!glfwWindowShouldClose(window)) {
		// Update interval = 60 times / second
		// renderer.clear();
		
		// Update logic every logic_update_interval
		if (glfwGetTime() - last_time > logic_update_interval) {
			last_time = glfwGetTime();
		}

		// Draw
		// renderer.draw(va, ib, glm::vec3(0, 0, 0));
		// renderer.draw(va, ib, glm::vec3(10, 10, 0));

		jactorio_renderer::draw();
		
		jactorio_renderer::update_shader_mvp();
		glfwSwapBuffers(window);  // Done rendering
		glfwPollEvents();
	}

	jactorio_renderer::terminate();
	jactorio_renderer::opengl_terminate();
}