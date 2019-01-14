#pragma once

#include "..\Context\Context.h"

namespace Engine {
	namespace Core{
		class GlobalManagers;
		class Game;
	}
	namespace Graphics {
		namespace Contexts {
			namespace Default {
				void glfwErrorCallback(int err_code, const char* info);
				GLvoid APIENTRY debugCallback(GLenum source,
											  GLenum type,
											  GLuint id,
											  GLenum severity,
											  GLsizei length,
											  const GLchar* message,
											  const void* userParam);
			}
		}
		namespace Managers {
			class Context {
			friend Core::GlobalManagers;
			friend Core::Game;
			protected:
				Context(const Graphics::Context::WindowInfo& default_context);
				~Context();
			private:
				bool resized = false;
				size_t context_count = 0;
				size_t context_cache = 0;
				Graphics::Context** Contexts = nullptr;
				size_t current = -1;
				Graphics::Context::WindowInfo display;
				size_t unload_count = 0;
				size_t* unloaded = nullptr;
				size_t unload_cache = 0;
			public:
				void Refresh(){
					glfwSwapBuffers(display.Window);
				}
				bool Resize(size_t width, size_t height, size_t ID = 0);
				size_t GenContext(const Graphics::Context::WindowInfo& info, GLFWwindow* shared=nullptr);
				size_t GetContext() {
					return current;
				};
				//currently not implemented
				size_t GetContext(GLFWwindow* window){
					return 0;
				}
				bool noContext(){
					return !context_count;
				}
				bool DelContext(size_t ID = 0);
				bool DelContext(GLFWwindow* window){
					return DelContext(GetContext(window));
				}
				bool SetContext(size_t ID = 0);
				bool SetContext(GLFWwindow* window){
					return SetContext(GetContext(window));
				}
				bool reserve(size_t size = 1);
				bool EnableDebug(GLDEBUGPROC debugCallback = (GLDEBUGPROC)Contexts::Default::debugCallback);
				GLFWwindowclosefun SetCloseCallback(size_t ID, GLFWwindowclosefun function);
				GLFWwindowrefreshfun SetRefreshCallback(size_t ID, GLFWwindowrefreshfun function);
				GLFWwindowfocusfun SetFocusCallback(size_t ID, GLFWwindowfocusfun function);
				GLFWwindowiconifyfun SetIconifyCallback(size_t ID, GLFWwindowiconifyfun function);
				GLFWwindowposfun SetPosCallback(size_t ID, GLFWwindowposfun function);
				GLFWwindowsizefun SetSizeCallback(size_t ID, GLFWwindowsizefun function);
				GLFWframebuffersizefun SetFBSizeCallback(size_t ID, GLFWwindowsizefun function);
			};
		}

	}
}
