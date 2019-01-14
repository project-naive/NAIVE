#include "Game.h"

#include <iostream>

namespace Engine {
	namespace Core {
		State* Game::state = nullptr;
		Game::Game(const Graphics::Context::WindowInfo& windowinfo):
			Managers(windowinfo){
			glViewport(0, 0, GLsizei(windowinfo.width), GLsizei(windowinfo.height));
			Active_Game = this;
		}

		Game::~Game() {
			if (state) {
				delete state;
			}
		}

		//Called from callbacks
		void Game::notifyClose(size_t contextID){
			Managers.ContextManager.DelContext(contextID);
			if(Managers.ContextManager.noContext()){
				glfwTerminate();
				running = false;
			}
			if(Managers.ContextManager.current == contextID){
				//switch to next context...
			}
		}

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
		Game* Active_Game = nullptr;
	}
}


