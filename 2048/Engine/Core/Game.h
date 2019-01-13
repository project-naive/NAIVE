#pragma once

#include "State.h"

#include <GL\glew.h>
#include <glfw\glfw3.h>

#include "..\Rendering\Managers\TextManager.h"
#include "..\Rendering\Managers\ShaderManager.h"
#include "..\Rendering\Managers\VertexAttributeManager.h"
#include "..\Rendering\Managers\ModelManager.h"

namespace Engine {
	namespace Core {
		class Game;
		typedef struct {
			GLFWwindow* Window     = nullptr;
			const char* game_title = nullptr;
			bool idle              = false;
			bool resizable         = true;
			size_t width           = 0;
			size_t height          = 0;
			bool fullscreen        = false;
			int monitor_count      = 0;
			GLFWmonitor** monitors = nullptr;
			GLFWwindowclosefun     Close   = nullptr;
			GLFWwindowrefreshfun   Refresh = nullptr;
			GLFWwindowfocusfun     Focus   = nullptr;
			GLFWwindowiconifyfun   Iconify = nullptr;
			GLFWwindowposfun       Pos     = nullptr;
			GLFWwindowsizefun      Size    = nullptr;
			GLFWframebuffersizefun FBSize  = nullptr;
		} WindowInfo;

		extern int argc_m;
		extern char** argv_m;
		extern char** env_m;

		class Game {
		protected:
			Game(const WindowInfo& info);
			~Game();
		public:
			static State* state;
			const size_t default_width;
			const size_t default_height;
			WindowInfo display;

			Rendering::Managers::Text* TextManager = nullptr;
			Rendering::Managers::Shader* ShaderManager = nullptr;
			Rendering::Managers::VertexAttribute* VertexAttributeManager = nullptr;
//			Other managers here
		};

		namespace Default {
			void glfwErrorCallback(int err_code, const char* info);
			void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
			namespace WindowCallback {
				void Close(GLFWwindow* window);
				void Refresh(GLFWwindow* window);
				void Focus(GLFWwindow* window, int flag);
				void Iconify(GLFWwindow* window, int flag);
				void Pos(GLFWwindow* window, int width, int height);
				void Size(GLFWwindow* window, int width, int height);
			}

			inline WindowInfo default_window() {
				WindowInfo rtn{};
				rtn.Close = WindowCallback::Close;
				rtn.Refresh = WindowCallback::Refresh;
				rtn.Focus = WindowCallback::Focus;
				rtn.Iconify = WindowCallback::Iconify;
				rtn.Pos = WindowCallback::Pos;
				rtn.Size = WindowCallback::Size;
				rtn.FBSize = FrameBufferSizeCallback;
				return rtn;
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
							  const void* userParam);
