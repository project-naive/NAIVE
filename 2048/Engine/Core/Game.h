#pragma once

#include "State.h"

#include <GL\glew.h>
#include <glfw\glfw3.h>

#include "..\Graphics\Managers\TextManager.h"
#include "..\Graphics\Managers\ShaderManager.h"
#include "..\Graphics\Managers\ModelManager.h"
#include "..\Graphics\Managers\ContextManager.h"

namespace Engine {
	namespace Core {
		extern int argc_m;
		extern char** argv_m;
		extern char** env_m;

		class GlobalManagers{
		public:
			GlobalManagers(Graphics::Context::WindowInfo info): ContextManager(info) {}
			Graphics::Managers::Context ContextManager;
			Graphics::Managers::Shader  ShaderManager;
			Graphics::Managers::Text    TextManager;
		};

		class Game {
		protected:
			Game(const Graphics::Context::WindowInfo& info);
			~Game();
		public:
			GlobalManagers Managers;
			static State* state;
			bool running = true;
			virtual bool ShouldEnd() {
				return glfwWindowShouldClose(Managers.ContextManager.display.Window);
			};
			//Called from callbacks
			virtual void notifyClose(size_t contextID = 0);
			virtual void notifyRefresh(size_t contextID = 0) {}
			virtual void notifyFocus(int flag, size_t contextID = 0) {}
			virtual void notifyIconify(int flag, size_t contextID = 0) {}
			virtual void notifyPos(int width, int height, size_t contextID = 0) {}
			virtual void notifySize(int width, int height, size_t contextID = 0) {}
			virtual void notifyFBSize(int width, int height, size_t contextID = 0) {
				Managers.ContextManager.Resize(width,height,contextID);
			};
		};
		extern Core::Game* Active_Game;
	}
}


