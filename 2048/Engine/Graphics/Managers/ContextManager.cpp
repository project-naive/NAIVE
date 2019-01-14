#include "ContextManager.h"

#include <iostream> 

namespace Engine {
	namespace Graphics {
		namespace Managers {
			Context::Context(const Graphics::Context::WindowInfo& default_context) {
				glfwSetErrorCallback(Contexts::Default::glfwErrorCallback);
				if (!glfwInit()) {
					std::cerr << "Error initializing glfw!\n" << std::endl;
					exit(EXIT_FAILURE);
				}
				Contexts = new Graphics::Context*[1];
				context_cache = 1;
				Contexts[0] = new Graphics::Context(default_context);
				context_count = 1;
				current = 0;
				display = Contexts[0]->display;
				if(default_context.debug) EnableDebug(nullptr);
			}
			Context::~Context() {
				for (size_t i = 0; i < context_count; i++) {
					delete Contexts[i];
				}
				delete[] Contexts;
				delete[] unloaded;
				glfwTerminate();
			}

			bool Context::Resize(size_t width, size_t height, size_t ID){
				if(ID >= context_count || !Contexts[ID]) return false;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				info.width = width;
				info.height = height;
				if(ID == current){
					display.width = width;
					display.height = height;
					glViewport(0, 0, GLsizei(height), GLsizei(width));
				} else{
					glfwMakeContextCurrent(info.Window);
					glViewport(0, 0, GLsizei(height), GLsizei(width));
					//						glfwSwapBuffers(info.Window);
					glfwMakeContextCurrent(display.Window);
				}
				return true;
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
					context_count++;
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
					context_count--;
				}
				return true;
			}

			bool Context::SetContext(size_t ID) {
				if(ID >= context_count || !Contexts[ID]) return false;
				if(Contexts[ID]->MakeCurrent()){
					current = ID;
					display = Contexts[ID]->display;
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

			bool Context::EnableDebug(GLDEBUGPROC debugCallback) {
				if(current==size_t(-1)) return false;
				glEnable(GL_DEBUG_OUTPUT);
				if(!debugCallback){
					debugCallback=(GLDEBUGPROC)Contexts::Default::debugCallback;
				}
				glDebugMessageCallback(debugCallback, Contexts[current]);
				return true;
			}

			GLFWwindowclosefun     Context::SetCloseCallback(size_t ID, GLFWwindowclosefun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowclosefun rtn = glfwSetWindowCloseCallback(info.Window, function);
				info.Close = function;
				return rtn;
			}
			GLFWwindowrefreshfun   Context::SetRefreshCallback(size_t ID, GLFWwindowrefreshfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowrefreshfun rtn = glfwSetWindowRefreshCallback(info.Window, function);
				info.Refresh = function;
				return rtn;
			}
			GLFWwindowfocusfun     Context::SetFocusCallback(size_t ID, GLFWwindowfocusfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowfocusfun rtn = glfwSetWindowFocusCallback(info.Window, function);
				info.Focus = function;
				return rtn;
			}
			GLFWwindowiconifyfun   Context::SetIconifyCallback(size_t ID, GLFWwindowiconifyfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowiconifyfun rtn = glfwSetWindowIconifyCallback(info.Window, function);
				info.Iconify = function;
				return rtn;
			}
			GLFWwindowposfun       Context::SetPosCallback(size_t ID, GLFWwindowposfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowposfun rtn = glfwSetWindowPosCallback(info.Window, function);
				info.Pos = function;
				return rtn;
			}
			GLFWwindowsizefun      Context::SetSizeCallback(size_t ID, GLFWwindowsizefun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowsizefun rtn = glfwSetWindowSizeCallback(info.Window, function);
				info.Size = function;
				return rtn;
			}
			GLFWframebuffersizefun Context::SetFBSizeCallback(size_t ID, GLFWwindowsizefun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWframebuffersizefun rtn = glfwSetFramebufferSizeCallback(info.Window, function);
				info.FBSize = function;
				return rtn;
			}
		}
		namespace Contexts {
			namespace Default {
				void glfwErrorCallback(int err_code, const char* info) {
					std::cerr << "Error " << err_code << ":\n" << info << '\n' << std::endl;
				}
				GLvoid APIENTRY debugCallback(GLenum source,
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
			}
		}
	}
}