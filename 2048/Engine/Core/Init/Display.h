#pragma once

#include <GL\glew.h>
#include <glfw\glfw3.h>

namespace Display {
	typedef void(*displayFunc)();

	extern displayFunc function;
	constexpr unsigned default_width = 512;
	constexpr unsigned default_height = 512;

	extern bool fullscreen;

	extern int monitor_count;
	extern GLFWmonitor** monitors;

	extern GLFWwindow* mainWindow;

	extern unsigned width;
	extern unsigned height;

	namespace Callback {
		namespace Window {
			void Close(GLFWwindow* window);
			void Refresh(GLFWwindow* window);
			//flag is GLFW_TRUE if iconified else GLFW_FALSE
			void Focus(GLFWwindow* window, int flag);
			void Iconify(GLFWwindow* window, int flag);
			void Pos(GLFWwindow* window, int width, int height);
			void Size(GLFWwindow* window, int width, int height);
		};
	}
}








