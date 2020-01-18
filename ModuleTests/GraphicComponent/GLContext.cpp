#include "GLContext.h"
#include "GLContextManager.h"

//#include "..\Managers\Globals\ContextManager.h"
//#include "..\..\Core\Game.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <glm\gtc\matrix_transform.hpp>

#include <iostream>

//#include <MI/mimalloc.h>

namespace Engine {
	namespace Graphics {
		std::atomic<size_t> volatile GLContext::id_max = 0;
		GLContext::GLContext(WindowInfo* info_ptr, GLContext* shared):
			ID(GLContext::id_max.fetch_add(1)){
			GLFWwindow* prev = glfwGetCurrentContext();
			std::clog << "Creating OpenGL Context!\n" << std::endl;
			bool is_default = false;
			if (!info_ptr) {
				display = Engine::Graphics::Contexts::Default::default_window();
				info_ptr = &display;
				is_default = true;
			}
			WindowInfo& info = *info_ptr;
			/*
			if (!info.Extended_Info.Display.Window.title)
				info.Extended_Info.Display.Window.title = "";
				*/
			info.Extended_Info.Display.Window.CreationInfo.minWidth =
				std::min(int(default_width), info.Extended_Info.Display.Window.CreationInfo.minWidth);
			info.Extended_Info.Display.Window.CreationInfo.minHeight =
				std::min(int(default_height), info.Extended_Info.Display.Window.CreationInfo.minHeight);
			{
				glfwWindowHint(GLFW_CONTEXT_CREATION_API,
					info.Extended_Info.GLContext.ContextInfo.Creation_API);
				glfwWindowHint(GLFW_CLIENT_API,
					info.Extended_Info.GLContext.ContextInfo.Client_API);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,
					info.Extended_Info.GLContext.ContextInfo.GL_Version_Major);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,
					info.Extended_Info.GLContext.ContextInfo.GL_Version_Minor);
				glfwWindowHint(GLFW_OPENGL_PROFILE,
					info.Extended_Info.GLContext.ContextInfo.OGLProfile);
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
					info.Extended_Info.GLContext.ContextInfo.OGLForward_Compat);
				glfwWindowHint(GLFW_CONTEXT_NO_ERROR,
					info.Extended_Info.GLContext.ContextInfo.No_Error);
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,
					info.Extended_Info.GLContext.ContextInfo.debug);
				glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS,
					info.Extended_Info.GLContext.ContextInfo.Robustness);
				glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR,
					info.Extended_Info.GLContext.ContextInfo.Release_Behavior);
			}
			{
				glfwWindowHint(GLFW_DOUBLEBUFFER,
					info.Basic_Info.double_buffer);
				glfwWindowHint(GLFW_STEREO,
					info.Extended_Info.GLContext.FramebufferInfo.Stereoscopic);
				glfwWindowHint(GLFW_SRGB_CAPABLE,
					info.Extended_Info.GLContext.FramebufferInfo.SRGB_Capable);
				glfwWindowHint(GLFW_RED_BITS,
					info.Extended_Info.GLContext.FramebufferInfo.Red_Bits);
				glfwWindowHint(GLFW_GREEN_BITS,
					info.Extended_Info.GLContext.FramebufferInfo.Green_Bits);
				glfwWindowHint(GLFW_BLUE_BITS,
					info.Extended_Info.GLContext.FramebufferInfo.Blue_Bits);
				glfwWindowHint(GLFW_ALPHA_BITS,
					info.Extended_Info.GLContext.FramebufferInfo.Alpha_Bits);
				glfwWindowHint(GLFW_DEPTH_BITS,
					info.Extended_Info.GLContext.FramebufferInfo.Depth_Bits);
				glfwWindowHint(GLFW_STENCIL_BITS,
					info.Extended_Info.GLContext.FramebufferInfo.Stencil_Bits);
				glfwWindowHint(GLFW_SAMPLES,
					info.Extended_Info.GLContext.FramebufferInfo.Samples);
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
			GLFWwindow* share = nullptr;
			if (shared)share = shared->display.Basic_Info.Window;
			if (info.Extended_Info.Display.Window.title) {
				info.Basic_Info.Window = glfwCreateWindow(
					int(info.Basic_Info.width),
					int(info.Basic_Info.height),
					info.Extended_Info.Display.Window.title,
					nullptr, share);
			}
			else {
				info.Basic_Info.Window = glfwCreateWindow(
					int(info.Basic_Info.width),
					int(info.Basic_Info.height),
					"",
					nullptr, share);
			}
			//			std::cout << mi_heap_check_owned(mi_heap_get_default(), info.Basic_Info.Window) << '\n';
			if (!info.Basic_Info.Window) {
				std::cerr << "Failed creating GLFW window \"" << info.Extended_Info.Display.Window.title
					<< "\"\nWidth:\t\t" << info.Basic_Info.width << "\nHeight:\t\t" << info.Basic_Info.height
					<< "\nFullScreen:\t" << info.Basic_Info.fullscreen << "\nResizable:\t" << info.Basic_Info.resizable
					<< "\nRequested OpenGL Version: " << info.Extended_Info.GLContext.ContextInfo.GL_Version_Major
					<< '.' << info.Extended_Info.GLContext.ContextInfo.GL_Version_Minor << '\n' << std::endl;
				glfwMakeContextCurrent(prev);
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
			}
			else {
				info.Extended_Info.Display.Window.CreationInfo.minWidth = -1;
				info.Extended_Info.Display.Window.CreationInfo.minHeight = -1;
				info.Extended_Info.Display.Window.CreationInfo.maxWidth = -1;
				info.Extended_Info.Display.Window.CreationInfo.maxHeight = -1;
			}

			if (info.Basic_Info.fullscreen) {
				//other things to do for full screen

			}
			{
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
			}
			glfwMakeContextCurrent(info.Basic_Info.Window);
			glfwSwapInterval(info.Extended_Info.Display.SwapInterval);
			glewExperimental = true;
			GLenum glewinit = glewInit();
			if (glewinit != GLEW_OK) {
				std::cerr << "Error initializing glew!:\n"
					<< glewGetErrorString(glewinit) << '\n' << std::endl;
				glfwMakeContextCurrent(prev);
				return;
			}
			std::clog << "GLEW initialized!\n" << std::endl;
			if (glewIsSupported(( std::string("GL_VERSION_")
								 + char('0' + info.Extended_Info.GLContext.ContextInfo.GL_Version_Major) + "_"
								 + char('0' + info.Extended_Info.GLContext.ContextInfo.GL_Version_Minor) ).c_str())) {
				std::clog << "OpenGL supports version "
					<< info.Extended_Info.GLContext.ContextInfo.GL_Version_Major << '.'
					<< info.Extended_Info.GLContext.ContextInfo.GL_Version_Minor << "!\n" << std::endl;
			} else {
				std::cerr << " OpenGL "
					<< info.Extended_Info.GLContext.ContextInfo.GL_Version_Major << '.'
					<< info.Extended_Info.GLContext.ContextInfo.GL_Version_Minor << " not supported!\n" << std::endl;
			}
			if (info.Extended_Info.GLContext.ContextInfo.debug) { 
				glEnable(GL_DEBUG_OUTPUT); 
				glDebugMessageCallback(Contexts::Default::debugCallback, this);
			}
			{
				default_width = info.Basic_Info.width;
				default_height = info.Basic_Info.height;
				projection = 
					glm::ortho(0.0f, float(info_ptr->Basic_Info.width),
						0.0f, float(info_ptr->Basic_Info.height));
				//has to be little endian system to work
				glfwGetFramebufferSize(info.Basic_Info.Window, &info.Basic_Info.width, &info.Basic_Info.height);
				info.Basic_Info.resizable 
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_RESIZABLE);
				info.Extended_Info.Display.Window.CreationInfo.auto_iconify 
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_AUTO_ICONIFY);
				info.Extended_Info.Display.Window.CreationInfo.decorated
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_DECORATED);
				info.Extended_Info.Display.Window.CreationInfo.floating
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_FLOATING);
				info.Extended_Info.Display.Monitor.Monitor
					= glfwGetWindowMonitor(info.Basic_Info.Window);
				info.Extended_Info.GLContext.ContextInfo.Client_API
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_CLIENT_API);
				info.Extended_Info.GLContext.ContextInfo.Creation_API
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_CONTEXT_CREATION_API);
				info.Extended_Info.GLContext.ContextInfo.GL_Version_Major
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_CONTEXT_VERSION_MAJOR);
				info.Extended_Info.GLContext.ContextInfo.GL_Version_Minor
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_CONTEXT_VERSION_MINOR);
				info.Extended_Info.GLContext.ContextInfo.debug
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_OPENGL_DEBUG_CONTEXT);
				info.Extended_Info.GLContext.ContextInfo.No_Error
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_CONTEXT_NO_ERROR);
				info.Extended_Info.GLContext.ContextInfo.OGLForward_Compat
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_OPENGL_FORWARD_COMPAT);
				info.Extended_Info.GLContext.ContextInfo.OGLProfile
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_OPENGL_PROFILE);
				info.Extended_Info.GLContext.ContextInfo.Release_Behavior
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_CONTEXT_RELEASE_BEHAVIOR);
				info.Extended_Info.GLContext.ContextInfo.Robustness
					= glfwGetWindowAttrib(info.Basic_Info.Window, GLFW_CONTEXT_ROBUSTNESS);
				glGetIntegerv(GL_RED_BITS, &info.Extended_Info.GLContext.FramebufferInfo.Red_Bits);
				glGetFramebufferAttachmentParameteriv(
					GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
					&info.Extended_Info.GLContext.FramebufferInfo.Red_Bits);
				glGetFramebufferAttachmentParameteriv(
					GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,
					&info.Extended_Info.GLContext.FramebufferInfo.Green_Bits);
				glGetFramebufferAttachmentParameteriv(
					GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,
					&info.Extended_Info.GLContext.FramebufferInfo.Blue_Bits);
				glGetFramebufferAttachmentParameteriv(
					GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
					&info.Extended_Info.GLContext.FramebufferInfo.Alpha_Bits);
				glGetFramebufferAttachmentParameteriv(
					GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
					&info.Extended_Info.GLContext.FramebufferInfo.Depth_Bits);
				glGetFramebufferAttachmentParameteriv(
					GL_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
					&info.Extended_Info.GLContext.FramebufferInfo.Stencil_Bits);
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			if (!is_default) {
				display = info;
			}
			glfwSetWindowUserPointer(info.Basic_Info.Window, this);
			glfwMakeContextCurrent(prev);
		}
		GLContext::~GLContext() {
			glfwDestroyWindow(display.Basic_Info.Window);
		}
		bool GLContext::MakeCurrent() {
			glfwMakeContextCurrent(display.Basic_Info.Window);
//			glViewport(0, 0, GLsizei(display.Basic_Info.width), GLsizei(display.Basic_Info.height));
			return true;
		}
		namespace Contexts {
			namespace Default {
				void FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {

				}
				inline GLvoid APIENTRY debugCallback(GLenum source,
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
				namespace WindowCallback {
					void Close(GLFWwindow* window){
						
					}
					void Refresh(GLFWwindow* window){
//						Core::Game& cur_game = *(Core::Active_Game);
//						cur_game.notifyRefresh(cur_game.Managers.ContextManager.GetContext(window));
						glfwSwapBuffers(window);
					}
					void Focus(GLFWwindow* window, int flag){

					}
					void Iconify(GLFWwindow* window, int flag){

					}
					void Pos(GLFWwindow* window, int width, int height){

					}
					void Size(GLFWwindow* window, int width, int height){

					}
				}
				namespace InputCallback {
					void Key(GLFWwindow * window, int key, int scancode, int action, int mods) {

					}
				}
			}
		}
	}
}

