#pragma once

#include "State.h"

#include <GL\glew.h>
#include <glfw\glfw3.h>

#include "..\Graphics\Managers\Globals\ContextManager.h"
#include "..\Graphics\Managers\Random\ShaderManager.h"
#include "..\Graphics\Managers\Random\ModelManager.h"
#include "..\Graphics\Managers\Random\TextManager.h"

namespace Engine {
	namespace Core {
		extern int argc_m;
		extern char** argv_m;
		extern char** env_m;
		extern Core::Game* Active_Game;

		class GlobalManagers {
		public:
			GlobalManagers(Graphics::WindowInfo& info);
			//Must be a Singleton as required by OpenGL Implementation
			Graphics::Managers::Context ContextManager;
			//This can have many instances, but one is much better than having many
			//This manager does not rely on a specific context, but sharing shader
			//uniforms are also shared across contexts (e.g. Binding of texture)
			Graphics::Managers::Shader  ShaderManager;
			//This can have many instances, but one is much better than having many
			//This manager does not need to rely on a specific context
			//as long as the context shares textures
			Graphics::Managers::Text    TextManager;
		};
		class Game {
		protected:
			Game(Graphics::WindowInfo& info);
			~Game();
			State* state = nullptr;
			bool running = true;
		public:
			GlobalManagers Managers;
			virtual void MainLoop() = 0;
			//Called from callbacks
			virtual void notifyClose(size_t contextID = 0) = 0;
			virtual void notifyRefresh(size_t contextID = 0) = 0;
			virtual void notifyFocus(int flag, size_t contextID = 0) = 0;
			virtual void notifyIconify(int flag, size_t contextID = 0) = 0;
			virtual void notifyPos(int width, int height, size_t contextID = 0) = 0;
			virtual void notifySize(int width, int height, size_t contextID = 0) = 0;
			virtual void notifyFBSize(int width, int height, size_t contextID = 0) = 0;
		};
	}
}


