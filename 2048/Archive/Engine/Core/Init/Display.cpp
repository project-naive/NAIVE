#include "Display.h"
#include "Global.h"

#include <iostream>

namespace Display {
	displayFunc function = nullptr;
	bool fullscreen = false;
	bool resizable = true;

	unsigned width = default_width;
	unsigned height = default_height;

	int monitor_count = 0;
	GLFWmonitor** monitors = nullptr;

	GLFWwindow* mainWindow = nullptr;

	namespace Callback {
		void Window::Close(GLFWwindow* window) {
			glfwDestroyWindow(window);
			glfwTerminate();
			exit(EXIT_SUCCESS);
		}

		void Window::Refresh(GLFWwindow* window) {
			if (function) function();
		}

		//flag is GLFW_TRUE if iconified else GLFW_FALSE
		void Window::Focus(GLFWwindow* window, int flag) {
			if(flag) {
				Global::idle = false;
			}//focus lost, pause game
			else{
				Global::idle = true;
			} //focus gained, continue game
		}

		void Window::Iconify(GLFWwindow* window, int flag) {
			if (flag) {
				Global::idle = true;
			}//Iconified, pause game
			else {
				Global::idle = false;
			} //continue game
		}

		void Window::Pos(GLFWwindow* window, int width, int height) {

		}

		void Window::Size(GLFWwindow* window, int width, int height) {
			//update window size
			//
		}
	}
}