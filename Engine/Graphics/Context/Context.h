#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

namespace Engine {
	namespace Graphics {
		namespace Managers {
			class Context;
		}
		class Context {
			friend Managers::Context;
		public:
			typedef struct {
				GLFWwindow* Window   = nullptr;
				size_t width         = 0;
				size_t height        = 0;
				bool fullscreen    = false;
				bool resizable     = true;
			} Window_Basic_Info;

			typedef struct {
				const char* title         = nullptr;
				unsigned GL_Version_Major = 3;
				unsigned GL_Version_Minor = 3;
				bool     double_buffer    = true;
				bool     debug            = true;
				size_t        Monitor_count = 0;
				GLFWmonitor** Monotors      = nullptr;
				GLFWwindowclosefun     Close   = nullptr;
				GLFWwindowrefreshfun   Refresh = nullptr;
				GLFWwindowfocusfun     Focus   = nullptr;
				GLFWwindowiconifyfun   Iconify = nullptr;
				GLFWwindowposfun       Pos     = nullptr;
				GLFWwindowsizefun      Size    = nullptr;
				GLFWframebuffersizefun FBSize  = nullptr;
				int    swap_interval = 1;
				bool   keep_aspect   = true;
				//currently ignored, planned for integrating
				//a choice pattern and parsing when conflicting
				size_t Min_width     = 0;
				size_t Min_height    = 0;
				size_t Max_width     = 0;
				size_t Max_height    = 0;
			} Window_Exended_Info;

			typedef struct  {
				Window_Basic_Info   Basic_Info{};
				Window_Exended_Info Extended_Info{};
			} WindowInfo;
		protected:
			//Not constant because of window hints
			//Context creation switches the current context to initialize glew
			//No switching back to make things faster. The manager knows such property
			//And manages the interaction
			Context(WindowInfo& info, GLFWwindow* shared=nullptr);
			~Context();
			WindowInfo display;
			const size_t default_width;
			const size_t default_height;
			const glm::mat4 projection;
			bool MakeCurrent();
		};
		namespace Contexts {
			namespace Default {
				void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
				namespace WindowCallback {
					void Close(GLFWwindow* window);
					void Refresh(GLFWwindow* window);
					void Focus(GLFWwindow* window, int flag);
					void Iconify(GLFWwindow* window, int flag);
					void Pos(GLFWwindow* window, int width, int height);
					void Size(GLFWwindow* window, int width, int height);
				}

				inline Context::WindowInfo default_window() {
					Context::WindowInfo rtn{};
					rtn.Extended_Info.Close   = WindowCallback::Close;
					rtn.Extended_Info.Refresh = WindowCallback::Refresh;
					rtn.Extended_Info.Focus   = WindowCallback::Focus;
					rtn.Extended_Info.Iconify = WindowCallback::Iconify;
					rtn.Extended_Info.Pos     = WindowCallback::Pos;
					rtn.Extended_Info.Size    = WindowCallback::Size;
					rtn.Extended_Info.FBSize  = FrameBufferSizeCallback;
					return rtn;
				}
			}
		}
	}
}

