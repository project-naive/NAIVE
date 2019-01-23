#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

#include <functional>

namespace Engine {
	namespace Graphics {
		typedef struct {
			typedef struct {
				GLFWwindow* Window = nullptr;
				size_t width = 0;
				size_t height = 0;
				bool fullscreen = false;
				bool resizable = true;
			} Window_Basic_Info;
			typedef struct {
				typedef struct {
					typedef struct {
						int  OGLProfile = GLFW_OPENGL_ANY_PROFILE;
						bool OGLForward_Compat = false;

						int  GL_Version_Major = 1;
						int  GL_Version_Minor = 0;
						bool debug = false;
						int  Client_API = GLFW_OPENGL_API;
						int  Creation_API = GLFW_NATIVE_CONTEXT_API;
						int  Robustness = GLFW_NO_ROBUSTNESS;
						int  Release_Behavior = GLFW_ANY_RELEASE_BEHAVIOR;
						bool No_Error = false;
					} Window_Creation_Context_Info;
					typedef struct {
						bool double_buffer = true;
						bool Stereoscopic = false;

						bool SRGB_Capable = false;
						int  Red_Bits = 8;
						int  Green_Bits = 8;
						int  Blue_Bits = 8;
						int  Alpha_Bits = 8;
						int  Depth_Bits = 24;
						int  Stencil_Bits = 8;
						int  Samples = 0;
					} Window_Creation_FrameBuffer_Info;
					Window_Creation_Context_Info     ContextInfo{};
					Window_Creation_FrameBuffer_Info FramebufferInfo{};
				} Context_Creation_Info;
				typedef struct {
					typedef struct {
						GLFWwindowclosefun     Close = nullptr;
						GLFWwindowrefreshfun   Refresh = nullptr;
						GLFWwindowfocusfun     Focus = nullptr;
						GLFWwindowiconifyfun   Iconify = nullptr;
						GLFWwindowposfun       Pos = nullptr;
						GLFWwindowsizefun      Size = nullptr;
						GLFWframebuffersizefun FBSize = nullptr;
					} Window_Callback_Info;
					typedef struct {
						GLFWkeyfun             Key = nullptr;
						GLFWcharfun            Char = nullptr;
						GLFWcharmodsfun        CharMods = nullptr;
						GLFWmousebuttonfun     MouseButton = nullptr;
						GLFWcursorposfun       CursorPos = nullptr;
						GLFWcursorenterfun     CursorEnter = nullptr;
						GLFWscrollfun          Scroll = nullptr;
						GLFWdropfun            Drop = nullptr;
					} Input_Callback_Info;
					Input_Callback_Info  Input{};
					Window_Callback_Info Window{};
				} Callback_Info;
				typedef struct {
					typedef struct {
						typedef struct {
							bool decorated    = true;
							bool auto_iconify = true;
							bool floating     = false;
							bool KeepAspect   = true;
							int  minWidth  = GLFW_DONT_CARE;
							int  minHeight = GLFW_DONT_CARE;
							int  maxWidth  = GLFW_DONT_CARE;
							int  maxHeight = GLFW_DONT_CARE;
							bool KeepSmall = true;
						} Window_Creation_Window_Info;
						typedef struct {
							bool visible   = true;
							bool focused   = true;
							bool maximized = false;
						} Window_Creation_Dynamic_Info;
						const char* title = nullptr;
						Window_Creation_Window_Info  CreationInfo;
						Window_Creation_Dynamic_Info StartInfo;
					} Window_Mode_Info;
					typedef struct {
						GLFWmonitor*   Monitor = nullptr;
						GLFWmonitorfun MonitorCallback = nullptr;
						int Refresh_Rate = GLFW_DONT_CARE;
					} Monitor_Mode_Info;
					Window_Mode_Info  Window{};
					Monitor_Mode_Info Monitor{};
					int               SwapInterval = 1;
				} Display_Mode_Info;
				Display_Mode_Info     Display;
				Callback_Info         Callback;
				Context_Creation_Info Context;
			} Window_Exended_Info;
			Window_Basic_Info   Basic_Info{};
			Window_Exended_Info Extended_Info{};
		} WindowInfo;
		namespace Managers {
			class Context;
		}
		class Context {
			friend Managers::Context;
		protected:
			//Not constant because of window hints
			//Context creation switches the current context to initialize glew
			//No switching back to make things faster. The manager knows such property
			//And manages the interaction
			Context(WindowInfo& info, size_t ID, GLFWwindow* shared=nullptr);
			~Context();
			WindowInfo display;
			const size_t ID;
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
				namespace InputCallback {
					void Key(GLFWwindow* window, int key, int scancode, int action, int mods);
				}
				inline WindowInfo default_window() {
					WindowInfo rtn{};
					rtn.Extended_Info.Callback.Window.Close   = WindowCallback::Close;
					rtn.Extended_Info.Callback.Window.Refresh = WindowCallback::Refresh;
					rtn.Extended_Info.Callback.Window.Focus   = WindowCallback::Focus;
					rtn.Extended_Info.Callback.Window.Iconify = WindowCallback::Iconify;
					rtn.Extended_Info.Callback.Window.Pos     = WindowCallback::Pos;
					rtn.Extended_Info.Callback.Window.Size    = WindowCallback::Size;
					rtn.Extended_Info.Callback.Window.FBSize  = FrameBufferSizeCallback;
					rtn.Extended_Info.Callback.Input.Key      = InputCallback::Key;
					return rtn;
				}
			}
		}
	}
}

