#pragma once

#include "..\..\Context\Context.h"

#include <glm\glm.hpp>

namespace Engine {
	namespace Core{
		class GlobalManagers;
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
			//TO DO
			//Keep track of shared instances
			class Context {
			friend Core::GlobalManagers;
			protected:
				//Not constant because of window hints
				Context(Graphics::WindowInfo& default_context);
				~Context();
			public:
				size_t GetCurrent() const {
					return current;
				}
				void GetCurrentResolution(size_t& width, size_t& height) const {
					width = display.width;
					height = display.height;
				}
				void GetCurrentDefaultResolution(size_t& width, size_t& height) const {
					width = d_width;
					height = d_height;
				}
				glm::mat4 GetProjection() const {
					return projection;
				}
				void Refresh() const {
					glfwSwapBuffers(display.Window);
				}
				bool Resize(size_t width, size_t height, size_t ID = 0);
				size_t GenContext(Graphics::WindowInfo& info, size_t shared_ID = -1);
				size_t GetContext(GLFWwindow* window) const {
					if (display.Window == window) return current;
					for (size_t i = 0; i < context_count; i++) {
						if (Contexts[i] && Contexts[i]->display.Basic_Info.Window == window) return i;
					}
					return -1;
				}
				bool noContext() const {
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
			private:
				bool resized = false;
				size_t context_count = 0;
				size_t context_cache = 0;
				Graphics::Context** Contexts = nullptr;
				size_t current = -1;
				Graphics::Window_Basic_Info display;
				size_t d_width = 0;
				size_t d_height = 0;
				glm::mat4 projection{};
				size_t  unload_count = 0;
				size_t  unload_cache = 0;
				size_t* unloaded     = nullptr;
			};
		}

	}
}
