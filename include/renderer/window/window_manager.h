#ifndef RENDERER_WINDOW_WINDOW_MANAGER_H
#define RENDERER_WINDOW_WINDOW_MANAGER_H

#include <GLFW/glfw3.h>

// https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2

/**
 * Manages the window in which the user sees
 */
namespace jactorio::renderer::window_manager
{
	/**
	 * Creates openGL context and window
	 * @return Non 0 if error occurred
	 */
	int init(int width, int height);
	
	/**
	 * Terminates openGL context and window
	 * @return Non 0 if error occurred
	 */
	int terminate();

	
	[[nodiscard]] bool is_fullscreen();
	void set_fullscreen(bool fullscreen);


	GLFWwindow* get_window();
	bool context_active();
}

#endif // RENDERER_WINDOW_WINDOW_MANAGER_H
