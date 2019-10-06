#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include <GLFW/glfw3.h>

namespace jactorio
{
	namespace renderer
	{
		void setup(GLFWwindow* window);
		void draw();
		void terminate();
	}
}

#endif // IMGUI_MANAGER_H