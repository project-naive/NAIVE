/* 
*	Singleton class that manages OpenGL VertexAttributes 
*	For syncing across Models with different indexes.
*	OpenGL is a state machine, thus a global coordination is required
*/

#pragma once

#include <GL\glew.h>

#include "Manager.h"

namespace Engine {
	namespace Rendering {
		namespace Managers {
			class VertexAttribute:public Manager {
			public:
				void Init() override {};
				void Destroy() override;
				static GLuint RequestAttribute();
				static bool RemoveAttribute(GLuint index);
			private:
				static GLuint used_count;
				static GLuint* unloaded;
				static GLuint unload_count;
				static GLuint unload_cache;
			};
		}
	}
}