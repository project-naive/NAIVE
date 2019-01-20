#include "Context.h"

#include "..\Managers\Globals\ContextManager.h"
#include "..\..\Core\Game.h"

#include <iostream>
#include <string>
#include <glm\gtc\matrix_transform.hpp>

namespace Engine {
	namespace Graphics {
		Context::Context(WindowInfo& info, GLFWwindow* shared):
			default_width(info.Basic_Info.width), 
			default_height(info.Basic_Info.height),
			projection(
				glm::ortho(0.0f, float(info.Basic_Info.width), 
						   0.0f, float(info.Basic_Info.height))) {
			if (!info.Extended_Info.title) info.Extended_Info.title = "";
			glfwWindowHint(GLFW_DOUBLEBUFFER, info.Extended_Info.double_buffer);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info.Extended_Info.GL_Version_Major);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info.Extended_Info.GL_Version_Minor);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, info.Extended_Info.debug);
			if (info.Basic_Info.fullscreen) {
				//other things to do for full screen
			} else {
				glfwWindowHint(GLFW_RESIZABLE, info.Basic_Info.resizable);
				info.Basic_Info.Window = glfwCreateWindow(int(info.Basic_Info.width), int(info.Basic_Info.height), info.Extended_Info.title, nullptr, shared);
				if (!info.Basic_Info.Window) {
					std::cerr << "Failed creating GLFW window"<< info.Extended_Info.title<<"!\n"
						<<"Width:\t"<< info.Basic_Info.width<<"Height:\t"<< info.Basic_Info.height<<'\n'
						<<"FullScreen:\t"<< info.Basic_Info.fullscreen<<"Resizable:\t"<< info.Basic_Info.resizable
						<<"Requested OpenGL Version: "<< info.Extended_Info.GL_Version_Major<<'.'<< info.Extended_Info.GL_Version_Minor<<'\n'<<std::endl;
					return;
				}
				if (info.Extended_Info.keep_aspect) {
					glfwSetWindowAspectRatio(info.Basic_Info.Window, int(default_width), int(default_height));
				}
			}
			glfwSetWindowCloseCallback(info.Basic_Info.Window, info.Extended_Info.Close);
			glfwSetWindowRefreshCallback(info.Basic_Info.Window, info.Extended_Info.Refresh);
			glfwSetWindowFocusCallback(info.Basic_Info.Window, info.Extended_Info.Focus);
			glfwSetWindowIconifyCallback(info.Basic_Info.Window, info.Extended_Info.Iconify);
			glfwSetWindowPosCallback(info.Basic_Info.Window, info.Extended_Info.Pos);
			glfwSetWindowSizeCallback(info.Basic_Info.Window, info.Extended_Info.Size);
			glfwSetFramebufferSizeCallback(info.Basic_Info.Window, info.Extended_Info.FBSize);
			//Add input initializations and callbacks here, eg keyboard and joystick here...
			glfwMakeContextCurrent(info.Basic_Info.Window);
			glfwSwapInterval(info.Extended_Info.swap_interval);
			glewExperimental = true;
			GLenum glewinit = glewInit();
			if (glewinit != GLEW_OK) {
				std::cerr << "Error initializing glew!:\n"
					<< glewGetErrorString(glewinit) << '\n' << std::endl;
				return;
			}
			if (glewIsSupported(( std::string("GL_VERSION_")
								 + char('0' + info.Extended_Info.GL_Version_Major) + "_"
								 + char('0' + info.Extended_Info.GL_Version_Minor) ).c_str())) {
				std::clog << "OpenGL supports version "
					<< info.Extended_Info.GL_Version_Major << '.'
					<< info.Extended_Info.GL_Version_Minor << "!\n" << std::endl;
			} else {
				std::cerr << " OpenGL "
					<< info.Extended_Info.GL_Version_Major << '.'
					<< info.Extended_Info.GL_Version_Minor << " not supported!\n" << std::endl;
			}
			if (info.Extended_Info.debug) { 
				glEnable(GL_DEBUG_OUTPUT); 
				glDebugMessageCallback((GLDEBUGPROC)Contexts::Default::debugCallback, this);
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			display = info;
		}
		Context::~Context() {
			glfwDestroyWindow(display.Basic_Info.Window);
		}
		bool Context::MakeCurrent() {
			glfwMakeContextCurrent(display.Basic_Info.Window);
//			glViewport(0, 0, GLsizei(display.Basic_Info.width), GLsizei(display.Basic_Info.height));
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


