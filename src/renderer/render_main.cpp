#include <GL/glew.h>

#include "renderer/render_main.h"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/gui/imgui_manager.h"

#include "renderer/opengl/renderer.h"
#include "renderer/opengl/window_manager.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/texture.h"

// Main draw loop is here
void renderer_main() {
	if (opengl_init() != 0)
		return;

	GLFWwindow* window = opengl_get_window();
	jactorio_imgui::setup(window);

	// Update game logic 60 times per second
	constexpr float logic_update_interval = 1.f / 60;

	// #################################################################
	// Enables transparency in textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	const Vertex_array va{};

	// Use index buffers to avoid repeating oneself
	const float x_offset = 0.f;
	const float y_offset = 0.f;
	float triangle_coords[] = {
		 0.f  + x_offset, 0.f + y_offset, 0.f,
		 32.f + x_offset, 0.f + y_offset, 0.f,
		 32.f + x_offset, 6.f + y_offset, 0.f,
		 0.f  + x_offset, 6.f + y_offset, 0.f,
	};

	const Vertex_buffer vb(triangle_coords, 4 * 3 * sizeof(float));
	va.add_buffer(vb, 3, 0);

	float tex_coords[] = {
		0.f, 1.f,  // bottom left
		1.f, 1.f,  // bottom right
		1.f, 0.f,  // upper right
		0.f, 0.f,  // upper left
	};
	const Vertex_buffer vb2(tex_coords, 4 * 2 * sizeof(float));
	va.add_buffer(vb2, 2, 1);
	
	// #################################################################

	// Index buffer for optimization, avoids redundant vertices
	unsigned int triangle_cord_indices[] = {
		0, 1, 2,
		2, 3, 0
	};
	const Index_buffer ib(triangle_cord_indices, 6);

	// #################################################################

	const Shader shader(
		std::vector<Shader_creation_input> {
			{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
			{"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER}
		}
	);
	shader.bind();
	jactorio::set_mvp_uniform_location(shader.get_uniform_location("u_model_view_projection_matrix"));

	// Upper left is 0, 0, bottom right is n,ns
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	//std::cout << m_viewport[0] << " " << m_viewport[1] << " " << m_viewport[2] << " " << m_viewport[3] << "\n";

	const unsigned int tile_width = 30;
	const glm::mat4 proj_mat = glm::ortho(
		0.f, static_cast<float>(m_viewport[2] / tile_width),
		static_cast<float>(m_viewport[3] / tile_width), 0.f,
		-1.f, 1.f);
	jactorio::setg_projection_matrix(proj_mat);
	
	// #################################################################

	// Loading textures
	const Texture texture("data/core/graphics/background-image-logo.png");
	texture.bind();

	// Texture is bound to slot 0 above, tell this to shader
	Shader::set_uniform_1i(shader.get_uniform_location("u_texture"), 0);

	// #################################################################
	
	const Renderer renderer;
	
	double last_time = 0.f;
	while (!glfwWindowShouldClose(window)) {
		// Update interval = 60 times / second
		renderer.clear();
		
		// Update logic every logic_update_interval
		if (glfwGetTime() - last_time > logic_update_interval) {
			last_time = glfwGetTime();
		}

		// Draw
		// TODO convert to batch draw call
		renderer.draw(va, ib, glm::vec3(0, 0, 0));
		renderer.draw(va, ib, glm::vec3(10, 10, 0));

		jactorio_imgui::draw();
		
		jactorio::update_shader_mvp();
		glfwSwapBuffers(window);  // Done rendering
		glfwPollEvents();
	}

	jactorio_imgui::terminate();
	opengl_terminate();
}