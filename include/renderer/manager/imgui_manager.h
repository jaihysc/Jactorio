#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer/opengl/shader.h"

namespace jactorio_renderer
{
	void setup(GLFWwindow* window);
	void draw();
	void terminate();
}

#endif // IMGUI_MANAGER_H