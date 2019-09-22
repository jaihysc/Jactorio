#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <GLFW/glfw3.h>

// Main openGL functions, initialize and exit

// Creates an openGL context and window
// Returns non 0 if an error occurred
int opengl_init();

// Terminates openGL window and context
// Returns non 0 if an error occurred
int opengl_terminate();

GLFWwindow* opengl_get_window();
bool opengl_get_context_active();

#endif // WINDOW_MANAGER_H