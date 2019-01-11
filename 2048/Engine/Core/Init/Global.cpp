#include "Global.h"
#include "Display.h"

#include <iostream>

namespace Global {
	int argc_m = 0;
	char** argv_m = nullptr;
	char** env_m = nullptr;

	bool idle = false;
	const char* game_title = "2048";

	void Init() {
		glfwSetErrorCallback(Callback::error);
		if (!glfwInit()) {
			std::cerr << "Error initializing glfw!" << std::endl;
			exit(EXIT_FAILURE);
		}
		if (Display::fullscreen) {
			Display::monitors = glfwGetMonitors(&Display::monitor_count);
			//other things to do with full screen context
			//	glfwSetMonitorCallback;
		} else {
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			Display::mainWindow = glfwCreateWindow(Display::width, Display::height, game_title, nullptr, nullptr);
			if (!Display::mainWindow) {
				std::cerr << "Error creating window!" << std::endl;
				glfwTerminate();
				exit(EXIT_FAILURE);
			}
			glfwSetWindowAspectRatio(Display::mainWindow, Display::default_width, Display::default_height);
			glfwSetWindowCloseCallback(Display::mainWindow, Display::Callback::Window::Close);
			glfwSetWindowRefreshCallback(Display::mainWindow, Display::Callback::Window::Refresh);
			glfwSetWindowFocusCallback(Display::mainWindow, Display::Callback::Window::Focus);
			glfwSetWindowIconifyCallback(Display::mainWindow, Display::Callback::Window::Iconify);
			glfwSetWindowPosCallback(Display::mainWindow, Display::Callback::Window::Pos);
			glfwSetWindowSizeCallback(Display::mainWindow, Display::Callback::Window::Size);
		}
		//Add input initializations here, eg keyboard and joystick here...

		glfwMakeContextCurrent(Display::mainWindow);
		glfwSwapInterval(1);
		glewExperimental = true;
		GLenum glewinit = glewInit();
		if (glewinit!=GLEW_OK) {
			std::cerr << "Error initializing glew!:\n"
				<< glewGetErrorString(glewinit) << std::endl;
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
		if (glewIsSupported("GL_VERSION_3_3")) {
			std::cout << "OpenGL supports version 3.3!\n ";
		} else {
			std::cout << " OpenGL 3.3 not supported!\n ";
		}
	}

	void CleanUp() {
		glfwTerminate();
	}

	namespace Callback {
		void error(int err_code, const char* info) {
			std::cerr << "Error " << err_code << ":\n" << info << std::endl;
		}
	}
}


