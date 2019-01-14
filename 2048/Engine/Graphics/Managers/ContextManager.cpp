#include "ContextManager.h"

#include <iostream> 

namespace Engine {
	namespace Graphics {
		namespace Managers {
			Context::Context() {
				glfwSetErrorCallback(Contexts::Default::glfwErrorCallback);
				if (!glfwInit()) {
					std::cerr << "Error initializing glfw!\n" << std::endl;
					exit(EXIT_FAILURE);
				}
				glEnable(GL_DEBUG_OUTPUT);
				glDebugMessageCallback((GLDEBUGPROC)Contexts::Default::debugCallback, nullptr);
			}
			Context::~Context() {
				for (size_t i = 0; i < context_count; i++) {
					delete Contexts[i];
				}
				delete[] Contexts;
				delete[] unloaded;
			}

			size_t Context::GenContext(
				const Graphics::Context::WindowInfo& info,
				GLFWwindow* shared) {
                //Invalid parameters
				if (!info.fullscreen && ((!info.width)||(!info.height))) return -1;
				if (unload_count) {
					size_t rtn = unloaded[unload_count-1];
					Contexts[rtn] = new Graphics::Context(info, shared);
					unload_count--;
					return rtn;
				}
				if (context_count == context_cache) {
					size_t new_size = size_t(context_cache*1.618);
					new_size++;
					Graphics::Context** new_cache = new Graphics::Context*[new_size];
					for(size_t i = 0; i < context_cache; i++){
						new_cache[i] = Contexts[i];
					}
					std::swap(Contexts, new_cache);
					context_cache = new_size;
					delete[] new_cache;
				}
				Contexts[context_count] = new Graphics::Context(info, shared);
				context_count++;
				return context_count-1;
			}

			bool Context::DelContext(size_t ID) {
				if(ID >= context_count) return false;
				if(Contexts[ID]){
					if(unload_count == unload_cache){
						size_t new_size = size_t(unload_cache*1.618);
						new_size++;
						size_t* new_cache = new size_t[new_size];
						for(size_t i = 0; i < unload_count; i++){
							new_cache[i]=unloaded[i];
						}
						std::swap(unloaded, new_cache);
						unload_cache = new_size;
						delete[] new_cache;
					}
					unloaded[unload_count] = ID;
					unload_count++;
					delete Contexts[ID];
					Contexts[ID] = nullptr;
				}
				return true;
			}

			bool Context::SetContext(size_t ID) {
				if(ID >= context_count || !Contexts[ID]) return false;
				if(Contexts[ID]->MakeCurrent()){
					current = ID;
					return true;
				}
				return false;
			}

			bool Context::reserve(size_t size) {
				if(size < context_count) return false;
				Graphics::Context** new_cache = new Graphics::Context*[size];
				for(size_t i = 0; i < context_count; i++){
					new_cache[i]=Contexts[i];
				}
				std::swap(Contexts, new_cache);
				context_cache = size;
				delete[] new_cache;
				return true;
			}
		}
	}
}