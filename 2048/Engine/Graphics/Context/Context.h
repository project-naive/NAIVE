#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>

namespace Engine {
	namespace Graphics {
		namespace Managers {
			class Context;
		}
		class Context {
			friend Managers::Context;
		public:
			typedef struct {
				GLFWwindow* Window        = nullptr;
				bool resizable            = true;
				bool idle                 = false;
				size_t width              = 0;
				size_t height             = 0;
				bool fullscreen           = false;
				const char* title         = nullptr;
				unsigned GL_Version_Major = 3;
				unsigned GL_Version_Minor = 3;
				int monitor_count         = 0;
				GLFWmonitor** monitors    = nullptr;
				GLFWwindowclosefun     Close   = nullptr;
				GLFWwindowrefreshfun   Refresh = nullptr;
				GLFWwindowfocusfun     Focus   = nullptr;
				GLFWwindowiconifyfun   Iconify = nullptr;
				GLFWwindowposfun       Pos     = nullptr;
				GLFWwindowsizefun      Size    = nullptr;
				GLFWframebuffersizefun FBSize  = nullptr;
			} WindowInfo;
		protected:
			Context(const WindowInfo& info, GLFWwindow* shared=nullptr);
			~Context();
			bool revised = false;
			WindowInfo display;
			const size_t default_width;
			const size_t default_height;
		public:
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
			}
		}
	}
}

