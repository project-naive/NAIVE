#pragma once

#include "..\Context\Context.h"

namespace Engine {
	namespace Graphics {
		namespace Managers {
			class Context {
			public:
				Context();
				~Context();
			private:
				size_t context_count = 0;
				size_t context_cache = 0;
				Graphics::Context** Contexts = nullptr;
				size_t current = -1;
				size_t unload_count = 0;
				size_t* unloaded = nullptr;
				size_t unload_cache = 0;
			public:
				size_t GenContext(const Graphics::Context::WindowInfo& info, GLFWwindow* shared=nullptr);
				size_t GetContext() {
					return current;
				};
				bool DelContext(size_t ID);
				bool SetContext(size_t ID);
				bool reserve(size_t size);
			};
		}
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
	}
}
