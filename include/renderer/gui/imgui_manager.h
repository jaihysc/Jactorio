#ifndef RENDERER_GUI_IMGUI_MANAGER_H
#define RENDERER_GUI_IMGUI_MANAGER_H

#include <GLFW/glfw3.h>

namespace jactorio::renderer::imgui_manager
{
	inline bool show_debug_menu = false;

	void setup(GLFWwindow* window);
	void imgui_draw();
	void imgui_terminate();
}

#endif // RENDERER_GUI_IMGUI_MANAGER_H
