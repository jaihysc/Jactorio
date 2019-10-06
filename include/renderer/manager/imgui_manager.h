#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include <GLFW/glfw3.h>

namespace jactorio
{
	namespace renderer
	{
		void setup(GLFWwindow* window);
		void imgui_draw();
		void imgui_terminate();
	}
}

#endif // IMGUI_MANAGER_H