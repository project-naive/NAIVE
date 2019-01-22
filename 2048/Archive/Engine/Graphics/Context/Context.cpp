#include "Context.h"

#include "..\..\Core\Game.h"

#include <iostream>
#include <string>

namespace Engine {
	namespace Graphics {
		Context::Context(const WindowInfo & info, GLFWwindow* shared):
			default_width(info.width), 
			default_height(info.height) {
			display = info;
			display.idle = false;
			if (!display.title) display.title = "";
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
			if (display.fullscreen) {
				display.monitors = glfwGetMonitors(&display.monitor_count);
				//other things to do for full screen
			} else {
				glfwWindowHint(GLFW_RESIZABLE, display.resizable);
				display.Window = glfwCreateWindow(int(display.width), int(display.height), display.title, nullptr, shared);
				if (!display.Window) {
					std::cerr << "Failed creating GLFW window"<<display.title<<"!\n"
						<<"Width:\t"<<display.width<<"Height:\t"<<display.height<<'\n'
						<<"FullScreen:\t"<<display.fullscreen<<"Resizable:\t"<<display.resizable
						<<"Requested OpenGL Version: "<<display.GL_Version_Major<<'.'<<display.GL_Version_Minor<<'\n'<<std::endl;
					return;
				}
				glfwSetWindowAspectRatio(display.Window, int(default_width), int(default_height));
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
				return;
			}
			if (glewIsSupported(( std::string("GL_VERSION_")
								 + char('0' + display.GL_Version_Major) + "_"
								 + char('0' + display.GL_Version_Minor) ).c_str())) {
				std::cout << "OpenGL supports version "
					<< display.GL_Version_Major << '.'
					<< display.GL_Version_Minor << "!\n" << std::endl;
			} else {
				std::cout << " OpenGL "
					<< display.GL_Version_Major << '.'
					<< display.GL_Version_Minor << " not supported!\n" << std::endl;
			}
		}
		Context::~Context() {
			glfwDestroyWindow(display.Window);
		}
		bool Context::MakeCurrent() {
			glfwMakeContextCurrent(display.Window);
			glViewport(0, 0, GLsizei(display.width), GLsizei(display.height));
			glLoadIdentity();
			return true;
		}
		namespace Contexts {
			namespace Default {
				void FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
					Core::Game& cur_game = *(Core::Active_Game);
					cur_game.notifyFBSize(width, height, cur_game.Managers.ContextManager.GetContext(window));
				}
				namespace WindowCallback {
					void Close(GLFWwindow* window){
						Core::Game& cur_game = *(Core::Active_Game);
						cur_game.notifyClose(cur_game.Managers.ContextManager.GetContext(window));
//						glfwDestroyWindow(window)
					}
					void Refresh(GLFWwindow* window){
//						Core::Game& cur_game = *(Core::Active_Game);
//						cur_game.notifyRefresh(cur_game.Managers.ContextManager.GetContext(window));
						glfwSwapBuffers(window);
					}
					void Focus(GLFWwindow* window, int flag){
						Core::Game& cur_game = *(Core::Active_Game);
						cur_game.notifyFocus(flag, cur_game.Managers.ContextManager.GetContext(window));
					}
					void Iconify(GLFWwindow* window, int flag){
						Core::Game& cur_game = *(Core::Active_Game);
						cur_game.notifyIconify(flag, cur_game.Managers.ContextManager.GetContext(window));
					}
					void Pos(GLFWwindow* window, int width, int height){
						Core::Game& cur_game = *(Core::Active_Game);
						cur_game.notifyPos(width, height, cur_game.Managers.ContextManager.GetContext(window));
					}
					void Size(GLFWwindow* window, int width, int height){
						Core::Game& cur_game = *(Core::Active_Game);
						cur_game.notifySize(width, height, cur_game.Managers.ContextManager.GetContext(window));
					}
				}
			}
		}
	}
}


