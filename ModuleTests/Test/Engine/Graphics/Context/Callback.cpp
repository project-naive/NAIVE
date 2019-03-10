#include "Callback.h"

#include <iostream>

namespace Engine {
	namespace Graphics {
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
				void FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
				
				};
				namespace WindowCallback {
					void Close(GLFWwindow* window) {
					
					};
					void Refresh(GLFWwindow* window) {
					
					};
					void Focus(GLFWwindow* window, int flag) {
					
					};
					void Iconify(GLFWwindow* window, int flag) {
					
					};
					void Pos(GLFWwindow* window, int width, int height) {
					
					};
					void Size(GLFWwindow* window, int width, int height) {
					
					};
				}
				namespace InputCallback {
					void Key(GLFWwindow* window, int key, int scancode, int action, int mods) {
					
					};
				}
			}
		}
	}
}
