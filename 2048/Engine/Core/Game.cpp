#include "Game.h"

#include <iostream>

namespace Engine {
	namespace Core {
		State* Game::state = nullptr;
		Game::Game(const WindowInfo& windowinfo):
			default_width(windowinfo.width),
			default_height(windowinfo.height) {
			display = windowinfo;
			display.idle = false;
			if (!display.game_title) display.game_title = "";
			glfwSetErrorCallback(Default::glfwErrorCallback);
			if (!glfwInit()) {
				std::cerr << "Error initializing glfw!\n" << std::endl;
				exit(EXIT_FAILURE);
			}
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
			if (display.fullscreen) {
				display.monitors = glfwGetMonitors(&display.monitor_count);
				//other things to do for full screen
			} else {
				glfwWindowHint(GLFW_RESIZABLE, display.resizable);
				display.Window = glfwCreateWindow(display.width, display.height, display.game_title, nullptr, nullptr);
				if (!display.Window) {
					std::cerr << "Error creating window!\n" << std::endl;
					glfwTerminate();
					exit(EXIT_FAILURE);
				}
				glfwSetWindowAspectRatio(display.Window, default_width, default_height);
			}
			glfwSetWindowCloseCallback(display.Window, display.Close);
			glfwSetWindowRefreshCallback(display.Window, display.Refresh);
			glfwSetWindowFocusCallback(display.Window, display.Focus);
			glfwSetWindowIconifyCallback(display.Window, display.Iconify);
			glfwSetWindowPosCallback(display.Window, display.Pos);
			glfwSetWindowSizeCallback(display.Window, display.Size);
			glfwSetFramebufferSizeCallback(display.Window, display.FBSize);
			//Add input initializations and callbacks here, eg keyboard and joystick here...
			glfwMakeContextCurrent(display.Window);
			glfwSwapInterval(1);
			glewExperimental = true;
			GLenum glewinit = glewInit();
			if (glewinit != GLEW_OK) {
				std::cerr << "Error initializing glew!:\n"
					<< glewGetErrorString(glewinit) << '\n' << std::endl;
				glfwTerminate();
				exit(EXIT_FAILURE);
			}
			if (glewIsSupported("GL_VERSION_4_4")) {
				std::cout << "OpenGL supports version 4.4!\n" << std::endl;
			} else {
				std::cout << " OpenGL 4.4 not supported!\n" << std::endl;
			}
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback((GLDEBUGPROC)debugCallback,nullptr);
			glViewport(0, 0, display.width, display.height);

			TextManager = new Rendering::Managers::Text;
			TextManager->Init();
			ShaderManager = new Rendering::Managers::Shader;
			ShaderManager->Init();
			VertexAttributeManager = new Rendering::Managers::VertexAttribute;
			VertexAttributeManager->Init();
			//other managers here
		}
		Game::~Game() {
			if (state) {
				state->Destroy();
				delete state;
			}
			glfwTerminate();
		}


GLDEBUGPROC;

		namespace Default {

			void glfwErrorCallback(int err_code, const char* info) {
				std::cerr << "Error " << err_code << ":\n" << info << '\n' << std::endl;
			}




			void FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {

			}
			//This function does not free resources on its own
			//Use a generic one from loader if necessary
			namespace WindowCallback {
				void Close(GLFWwindow* window) {
					glfwDestroyWindow(window);
					glfwTerminate();
					exit(EXIT_SUCCESS);
				}

				void Refresh(GLFWwindow* window) {
					glfwSwapBuffers(window);
				}

				void Focus(GLFWwindow* window, int flag) {

				}

				void Iconify(GLFWwindow* window, int flag) {

				}

				void Pos(GLFWwindow* window, int width, int height) {

				}

				void Size(GLFWwindow* window, int width, int height) {

				}
			}
		}
	}

}

GLvoid APIENTRY debugCallback(GLenum source,
							  GLenum type,
							  GLuint id,
							  GLenum severity,
							  GLsizei length,
							  const GLchar* message,
							  const void* userParam) {
	std::cerr << "OpenGL Error occured!\n\nType:\t\t" << unsigned(type) 
		<< "\nID:\t\t" << unsigned(id) 
		<< "\nSeverity:\t" << unsigned(severity) 
		<< "\nLength:\t" << int(length)
		<< "\nMessage:\n" << (char*)message << '\n' << std::endl;
}
