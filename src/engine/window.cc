#include "window.hh"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

GLFWwindow *window;

void engine::init_window() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(800, 600, "nv", 0, 0);
	if (!window)
		throw std::runtime_error("failed to create window.");
	
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		throw std::runtime_error("glewInit failed.");
}

void engine::cleanup_window() {
	glfwTerminate();
}

void engine::update_window() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool engine::window_closed() {
	return glfwWindowShouldClose(window);
}
