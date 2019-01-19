#include "Game.h"

#include <iostream>

namespace Engine {
	namespace Core {
		Game* Active_Game = nullptr;
		GlobalManagers::GlobalManagers(Engine::Graphics::WindowInfo& info):
			ContextManager(info),
			ShaderManager(ContextManager),
			TextManager(ContextManager, ShaderManager) {}
		Game::Game(Graphics::WindowInfo& windowinfo):
			Managers(windowinfo){
			glViewport(0, 0, GLsizei(windowinfo.Basic_Info.width), GLsizei(windowinfo.Basic_Info.height));
			Active_Game = this;
		}

		Game::~Game() {
			if (state) {
				state->Destroy();
				delete state;
			}
		}
/*
		//Called from callbacks
		void Game::notifyClose(size_t contextID){

			state->notifyContextClose(contextID);
			Managers.ContextManager.DelContext(contextID);
			if(Managers.ContextManager.noContext()){
				glfwTerminate();
				state->running = false;
				running = false;
			}
//			if(Managers.ContextManager.current == contextID) {
				//switch to next context...
//			}

		}
*/
	}
}


