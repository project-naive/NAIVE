#pragma once

#include "WindowInfo.h"

namespace Engine {
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
					rtn.Extended_Info.Callback.Window.Close = WindowCallback::Close;
					rtn.Extended_Info.Callback.Window.Refresh = WindowCallback::Refresh;
					rtn.Extended_Info.Callback.Window.Focus = WindowCallback::Focus;
					rtn.Extended_Info.Callback.Window.Iconify = WindowCallback::Iconify;
					rtn.Extended_Info.Callback.Window.Pos = WindowCallback::Pos;
					rtn.Extended_Info.Callback.Window.Size = WindowCallback::Size;
					rtn.Extended_Info.Callback.Window.FBSize = FrameBufferSizeCallback;
					rtn.Extended_Info.Callback.Input.Key = InputCallback::Key;
					return rtn;
				}
			}
		}
	}
}

