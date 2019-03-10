#pragma once

#include "WindowInfo.h"
#include <glm/glm.hpp>

namespace Engine {
	namespace Graphics {
		class Context {
//			friend Managers::Context;
		protected:
			//Not constant because of window hints
			//Context creation switches the current context to initialize glew
			//No switching back to make things faster. The manager knows such property
			//And manages the interaction
			Context(WindowInfo& info, size_t ID, GLFWwindow* shared = nullptr);
			~Context();
			WindowInfo display;
			const size_t ID;
			const size_t default_width;
			const size_t default_height;
			const glm::mat4 projection;
			bool MakeCurrent();
		};
	}
}