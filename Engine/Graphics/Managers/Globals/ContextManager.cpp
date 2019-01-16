#include "ContextManager.h"

#include <iostream> 

namespace Engine {
	namespace Graphics {
		namespace Managers {
			Context::Context(Graphics::Context::WindowInfo& default_context) {
				glfwSetErrorCallback(Contexts::Default::glfwErrorCallback);
				if (!glfwInit()) {
					std::cerr << "Error initializing glfw!\n" << std::endl;
					exit(EXIT_FAILURE);
				}
				std::clog << "GLFW Initialized!\n" << std::endl;
				Contexts = new Graphics::Context*[1];
				context_cache = 1;
				Contexts[0] = new Graphics::Context(default_context);
				context_count = 1;
				current = 0;
				display = Contexts[0]->display.Basic_Info;
				d_width = Contexts[0]->default_width;
				d_height = Contexts[0]->default_height;
				projection = Contexts[0]->projection;
				if(default_context.Extended_Info.debug) EnableDebug();
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
				if(ID == current){
					display.width = width;
					display.height = height;
					glViewport(0, 0, GLsizei(height), GLsizei(width));
				} else {
					Graphics::Context::Window_Basic_Info& info = Contexts[ID]->display.Basic_Info;
					info.width = width;
					info.height = height;
					glfwMakeContextCurrent(info.Window);
					glViewport(0, 0, GLsizei(height), GLsizei(width));
					//						glfwSwapBuffers(info.Window);
					glfwMakeContextCurrent(display.Window);
				}
				return true;
			}

			size_t Context::GenContext(
				Graphics::Context::WindowInfo& info, size_t shared_ID) {
                //Invalid parameters
				if (!info.Basic_Info.fullscreen && ((!info.Basic_Info.width)||(!info.Basic_Info.height))) return -1;
				GLFWwindow* shared = nullptr;
				if ((shared_ID == size_t(-1) && current != size_t(-1))
					||shared_ID == current) shared = display.Window;
				else {
					if (!Contexts || !Contexts[shared_ID]) return -1;
					shared = Contexts[shared_ID]->display.Basic_Info.Window;
				}
				if (unload_count) {
					size_t rtn = unloaded[unload_count-1];
					Contexts[rtn] = new Graphics::Context(info, shared);
					unload_count--;
					context_count++;
					current = rtn;
					display = Contexts[rtn]->display.Basic_Info;
					d_width = Contexts[rtn]->default_width;
					d_height = Contexts[rtn]->default_height;
					projection = Contexts[rtn]->projection;
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
				current = context_count;
				display = Contexts[current]->display.Basic_Info;
				d_width = Contexts[current]->default_width;
				d_height = Contexts[current]->default_height;
				projection = Contexts[current]->projection;
				context_count++;
				return current;
			}

			bool Context::DelContext(size_t ID) {
				if(ID >= context_count) return false;
				//Cannot delete current one for now
				if (ID == current) current = -1;
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
					return true;
				}
				return false;
			}

			bool Context::SetContext(size_t ID) {
				if(ID >= context_count || !Contexts[ID]) return false;
				if (ID == current) return true;
				Contexts[current]->display.Basic_Info = display;
				if(Contexts[ID]->MakeCurrent()){
					current = ID;
					display = Contexts[ID]->display.Basic_Info;
					d_width = Contexts[ID]->default_width;
					d_height = Contexts[ID]->default_height;
					projection = Contexts[ID]->projection;
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
				if(!debugCallback) {
					debugCallback = (GLDEBUGPROC) Contexts::Default::debugCallback;
				}
				glDebugMessageCallback(debugCallback, Contexts[current]);
				return true;
			}

			GLFWwindowclosefun     Context::SetCloseCallback(size_t ID, GLFWwindowclosefun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowclosefun rtn = glfwSetWindowCloseCallback(info.Basic_Info.Window, function);
				info.Extended_Info.Close = function;
				return rtn;
			}
			GLFWwindowrefreshfun   Context::SetRefreshCallback(size_t ID, GLFWwindowrefreshfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowrefreshfun rtn = glfwSetWindowRefreshCallback(info.Basic_Info.Window, function);
				info.Extended_Info.Refresh = function;
				return rtn;
			}
			GLFWwindowfocusfun     Context::SetFocusCallback(size_t ID, GLFWwindowfocusfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowfocusfun rtn = glfwSetWindowFocusCallback(info.Basic_Info.Window, function);
				info.Extended_Info.Focus = function;
				return rtn;
			}
			GLFWwindowiconifyfun   Context::SetIconifyCallback(size_t ID, GLFWwindowiconifyfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowiconifyfun rtn = glfwSetWindowIconifyCallback(info.Basic_Info.Window, function);
				info.Extended_Info.Iconify = function;
				return rtn;
			}
			GLFWwindowposfun       Context::SetPosCallback(size_t ID, GLFWwindowposfun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowposfun rtn = glfwSetWindowPosCallback(info.Basic_Info.Window, function);
				info.Extended_Info.Pos = function;
				return rtn;
			}
			GLFWwindowsizefun      Context::SetSizeCallback(size_t ID, GLFWwindowsizefun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWwindowsizefun rtn = glfwSetWindowSizeCallback(info.Basic_Info.Window, function);
				info.Extended_Info.Size = function;
				return rtn;
			}
			GLFWframebuffersizefun Context::SetFBSizeCallback(size_t ID, GLFWwindowsizefun function){
				if(ID >= context_count || !Contexts[ID]) return nullptr;
				Graphics::Context::WindowInfo& info = Contexts[ID]->display;
				GLFWframebuffersizefun rtn = glfwSetFramebufferSizeCallback(info.Basic_Info.Window, function);
				info.Extended_Info.FBSize = function;
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