#pragma once

#include <GL\glew.h>
#include <glfw\glfw3.h>

namespace Display {
	typedef void(*displayFunc)();

	extern displayFunc function;

	//Default game window size (no scaling performed)
	constexpr unsigned default_width = 512;
	constexpr unsigned default_height = 512;

	//Parameters for current main display
	extern GLFWwindow* mainWindow;
	extern unsigned width;
	extern unsigned height;

	//windowed diplay parameters
	extern bool resizable;

	//fullscreen diplay parameters
	extern bool fullscreen;
	extern int monitor_count;
	extern GLFWmonitor** monitors;

	namespace Callback {
		//Window callback functions
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








