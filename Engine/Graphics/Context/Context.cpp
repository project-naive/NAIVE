#include "Context.h"

#include "..\Managers\Globals\ContextManager.h"
#include "..\..\Core\Game.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <glm\gtc\matrix_transform.hpp>

namespace Engine {
	namespace Graphics {
		Context::Context(WindowInfo& info, size_t given_ID,GLFWwindow* shared):
			ID(given_ID),
			default_width(info.Basic_Info.width), 
			default_height(info.Basic_Info.height),
			projection(
				glm::ortho(0.0f, float(info.Basic_Info.width), 
						   0.0f, float(info.Basic_Info.height))) {
			std::clog << "Creating OpenGL Context!\n" << std::endl;
			if (!info.Extended_Info.Display.Window.title) 
				info.Extended_Info.Display.Window.title = "";
			info.Extended_Info.Display.Window.CreationInfo.minWidth =
				std::min(int(default_width), info.Extended_Info.Display.Window.CreationInfo.minWidth);
			info.Extended_Info.Display.Window.CreationInfo.minHeight =
				std::min(int(default_height), info.Extended_Info.Display.Window.CreationInfo.minHeight);
			{
				glfwWindowHint(GLFW_CONTEXT_CREATION_API,
							   info.Extended_Info.Context.ContextInfo.Creation_API);
				glfwWindowHint(GLFW_CLIENT_API,
							   info.Extended_Info.Context.ContextInfo.Client_API);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,
							   info.Extended_Info.Context.ContextInfo.GL_Version_Major);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,
							   info.Extended_Info.Context.ContextInfo.GL_Version_Minor);
				glfwWindowHint(GLFW_OPENGL_PROFILE,
							   info.Extended_Info.Context.ContextInfo.OGLProfile);
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
							   info.Extended_Info.Context.ContextInfo.OGLForward_Compat);
				glfwWindowHint(GLFW_CONTEXT_NO_ERROR,
							   info.Extended_Info.Context.ContextInfo.No_Error);
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,
							   info.Extended_Info.Context.ContextInfo.debug);
				glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS,
							   info.Extended_Info.Context.ContextInfo.Robustness);
				glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR,
							   info.Extended_Info.Context.ContextInfo.Release_Behavior);
			}
			{
				glfwWindowHint(GLFW_DOUBLEBUFFER,
							   info.Extended_Info.Context.FramebufferInfo.double_buffer);
				glfwWindowHint(GLFW_STEREO,
							   info.Extended_Info.Context.FramebufferInfo.Stereoscopic);
				glfwWindowHint(GLFW_SRGB_CAPABLE,
							   info.Extended_Info.Context.FramebufferInfo.SRGB_Capable);
				glfwWindowHint(GLFW_RED_BITS,
							   info.Extended_Info.Context.FramebufferInfo.Red_Bits);
				glfwWindowHint(GLFW_GREEN_BITS,
							   info.Extended_Info.Context.FramebufferInfo.Green_Bits);
				glfwWindowHint(GLFW_BLUE_BITS,
							   info.Extended_Info.Context.FramebufferInfo.Blue_Bits);
				glfwWindowHint(GLFW_ALPHA_BITS,
							   info.Extended_Info.Context.FramebufferInfo.Alpha_Bits);
				glfwWindowHint(GLFW_DEPTH_BITS,
							   info.Extended_Info.Context.FramebufferInfo.Depth_Bits);
				glfwWindowHint(GLFW_STENCIL_BITS,
							   info.Extended_Info.Context.FramebufferInfo.Stencil_Bits);
				glfwWindowHint(GLFW_SAMPLES,
							   info.Extended_Info.Context.FramebufferInfo.Samples);
			}
			{
				glfwWindowHint(GLFW_RESIZABLE,
							   info.Basic_Info.resizable);
				glfwWindowHint(GLFW_FLOATING,
							   info.Extended_Info.Display.Window.CreationInfo.floating);
				glfwWindowHint(GLFW_DECORATED,
							   info.Extended_Info.Display.Window.CreationInfo.decorated);
				glfwWindowHint(GLFW_AUTO_ICONIFY,
							   info.Extended_Info.Display.Window.CreationInfo.auto_iconify);
				glfwWindowHint(GLFW_VISIBLE,
							   info.Extended_Info.Display.Window.StartInfo.visible);
				glfwWindowHint(GLFW_FOCUSED,
							   info.Extended_Info.Display.Window.StartInfo.focused);
				glfwWindowHint(GLFW_MAXIMIZED,
							   info.Extended_Info.Display.Window.StartInfo.maximized);
			}
			info.Basic_Info.Window = glfwCreateWindow(
				int(info.Basic_Info.width),
				int(info.Basic_Info.height),
				info.Extended_Info.Display.Window.title,
				nullptr, shared);
			if (!info.Basic_Info.Window) {
				std::cerr << "Failed creating GLFW window \"" << info.Extended_Info.Display.Window.title
					<< "\"\nWidth:\t\t" << info.Basic_Info.width << "\nHeight:\t\t" << info.Basic_Info.height
					<< "\nFullScreen:\t" << info.Basic_Info.fullscreen << "\nResizable:\t" << info.Basic_Info.resizable
					<< "\nRequested OpenGL Version: " << info.Extended_Info.Context.ContextInfo.GL_Version_Major
					<< '.' << info.Extended_Info.Context.ContextInfo.GL_Version_Minor << '\n' << std::endl;
				return;
			}
			if (info.Extended_Info.Display.Window.CreationInfo.KeepSmall) {
				glfwMaximizeWindow(info.Basic_Info.Window);
				int maximize_width, maximize_height;
				glfwGetFramebufferSize(info.Basic_Info.Window, &maximize_width, &maximize_height);
				glfwRestoreWindow(info.Basic_Info.Window);
				info.Extended_Info.Display.Window.CreationInfo.maxWidth =
					std::min(info.Extended_Info.Display.Window.CreationInfo.maxWidth,
							 maximize_width);
				info.Extended_Info.Display.Window.CreationInfo.maxHeight = 
					std::min(info.Extended_Info.Display.Window.CreationInfo.maxHeight, 
							 maximize_height);
				if (!info.Extended_Info.Display.Window.StartInfo.visible) glfwHideWindow(info.Basic_Info.Window);
			}
			if (info.Extended_Info.Display.Window.CreationInfo.KeepAspect) {
				double ratio = double(default_width) / default_height;
				info.Extended_Info.Display.Window.CreationInfo.minWidth =
					int(std::max(double(info.Extended_Info.Display.Window.CreationInfo.minWidth),
									ratio*info.Extended_Info.Display.Window.CreationInfo.minHeight));
				info.Extended_Info.Display.Window.CreationInfo.minHeight =
					int(std::max(double(info.Extended_Info.Display.Window.CreationInfo.minHeight),
									info.Extended_Info.Display.Window.CreationInfo.minWidth / ratio));
				info.Extended_Info.Display.Window.CreationInfo.maxWidth =
					int(std::min(double(info.Extended_Info.Display.Window.CreationInfo.maxWidth),
									ratio*info.Extended_Info.Display.Window.CreationInfo.maxHeight));
				info.Extended_Info.Display.Window.CreationInfo.maxHeight =
					int(std::min(double(info.Extended_Info.Display.Window.CreationInfo.maxHeight),
									info.Extended_Info.Display.Window.CreationInfo.maxWidth / ratio));
				glfwSetWindowAspectRatio(info.Basic_Info.Window, int(default_width), int(default_height));
			}
			if (info.Extended_Info.Display.Window.CreationInfo.minWidth >= 0
				&& info.Extended_Info.Display.Window.CreationInfo.minHeight >= 0
				&& info.Extended_Info.Display.Window.CreationInfo.minWidth 
				< info.Extended_Info.Display.Window.CreationInfo.maxWidth
				&& info.Extended_Info.Display.Window.CreationInfo.minHeight 
				< info.Extended_Info.Display.Window.CreationInfo.maxHeight) {
				glfwSetWindowSizeLimits(info.Basic_Info.Window,
										info.Extended_Info.Display.Window.CreationInfo.minWidth,
										info.Extended_Info.Display.Window.CreationInfo.minHeight,
										info.Extended_Info.Display.Window.CreationInfo.maxWidth,
										info.Extended_Info.Display.Window.CreationInfo.maxHeight);
			} else {
				info.Extended_Info.Display.Window.CreationInfo.minWidth = -1;
				info.Extended_Info.Display.Window.CreationInfo.minHeight = -1;
				info.Extended_Info.Display.Window.CreationInfo.maxWidth = -1;
				info.Extended_Info.Display.Window.CreationInfo.maxHeight = -1;
			}
			
			if (info.Basic_Info.fullscreen) {
				//other things to do for full screen

			}

			glfwSetWindowCloseCallback(info.Basic_Info.Window, 
									   info.Extended_Info.Callback.Window.Close);
			glfwSetWindowRefreshCallback(info.Basic_Info.Window, 
										 info.Extended_Info.Callback.Window.Refresh);
			glfwSetWindowFocusCallback(info.Basic_Info.Window, 
									   info.Extended_Info.Callback.Window.Focus);
			glfwSetWindowIconifyCallback(info.Basic_Info.Window, 
										 info.Extended_Info.Callback.Window.Iconify);
			glfwSetWindowPosCallback(info.Basic_Info.Window, 
									 info.Extended_Info.Callback.Window.Pos);
			glfwSetWindowSizeCallback(info.Basic_Info.Window,
									  info.Extended_Info.Callback.Window.Size);
			glfwSetFramebufferSizeCallback(info.Basic_Info.Window, 
										   info.Extended_Info.Callback.Window.FBSize);
			//Add input initializations and callbacks here, eg keyboard and joystick here...
			glfwSetKeyCallback(info.Basic_Info.Window, 
							   info.Extended_Info.Callback.Input.Key);
			glfwSetCharCallback(info.Basic_Info.Window, 
								info.Extended_Info.Callback.Input.Char);
			glfwSetCharModsCallback(info.Basic_Info.Window, 
									info.Extended_Info.Callback.Input.CharMods);
			glfwSetMouseButtonCallback(info.Basic_Info.Window, 
									   info.Extended_Info.Callback.Input.MouseButton);
			glfwSetCursorPosCallback(info.Basic_Info.Window, 
									 info.Extended_Info.Callback.Input.CursorPos);
			glfwSetCursorEnterCallback(info.Basic_Info.Window,
									   info.Extended_Info.Callback.Input.CursorEnter);
			glfwSetScrollCallback(info.Basic_Info.Window,
								  info.Extended_Info.Callback.Input.Scroll);
			glfwSetDropCallback(info.Basic_Info.Window, 
								info.Extended_Info.Callback.Input.Drop);
			glfwMakeContextCurrent(info.Basic_Info.Window);
			glfwSwapInterval(info.Extended_Info.Display.SwapInterval);
			glewExperimental = true;
			GLenum glewinit = glewInit();
			if (glewinit != GLEW_OK) {
				std::cerr << "Error initializing glew!:\n"
					<< glewGetErrorString(glewinit) << '\n' << std::endl;
				return;
			}
			std::clog << "GLEW initialized!\n" << std::endl;
			if (glewIsSupported(( std::string("GL_VERSION_")
								 + char('0' + info.Extended_Info.Context.ContextInfo.GL_Version_Major) + "_"
								 + char('0' + info.Extended_Info.Context.ContextInfo.GL_Version_Minor) ).c_str())) {
				std::clog << "OpenGL supports version "
					<< info.Extended_Info.Context.ContextInfo.GL_Version_Major << '.'
					<< info.Extended_Info.Context.ContextInfo.GL_Version_Minor << "!\n" << std::endl;
			} else {
				std::cerr << " OpenGL "
					<< info.Extended_Info.Context.ContextInfo.GL_Version_Major << '.'
					<< info.Extended_Info.Context.ContextInfo.GL_Version_Minor << " not supported!\n" << std::endl;
			}
			if (info.Extended_Info.Context.ContextInfo.debug) { 
				glEnable(GL_DEBUG_OUTPUT); 
				glDebugMessageCallback((GLDEBUGPROC)Contexts::Default::debugCallback, this);
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			display = info;
			glfwSetWindowUserPointer(info.Basic_Info.Window, this);
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
				namespace InputCallback {
					void Key(GLFWwindow * window, int key, int scancode, int action, int mods) {
						Core::Game& cur_game = *( Core::Active_Game );
						cur_game.notifyKey(key, scancode, action, mods, cur_game.Managers.ContextManager.GetContext(window));
					}
				}
			}
		}
	}
}

