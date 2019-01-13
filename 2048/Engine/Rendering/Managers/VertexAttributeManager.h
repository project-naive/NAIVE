#pragma once

#include <GL\glew.h>

namespace Rendering {
	namespace Manager {
		class VertexAttribute {
		public:
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
